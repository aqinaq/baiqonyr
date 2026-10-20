#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#undef GLAD_GL_IMPLEMENTATION
#include "Capture.hpp"
#include "RenderTarget.hpp"
#include "UI.hpp"
#include <GLFW/glfw3.h>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <memory>
#include <numeric>
#include <thread>
using namespace bk;
struct Options {
    int width = 1280, height = 720;
    double duration = 0, captureAt = -1;
    float startTime = -8;
    bool demo = false, launch = false, vsync = true;
    std::string capture = "output/screenshot.ppm", csv, record, ffmpeg = "ffmpeg";
};
Options options(int argc, char **argv) {
    Options o;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        auto value = [&]() {
            if (++i >= argc)
                throw std::runtime_error("Missing value for " + a);
            return std::string(argv[i]);
        };
        if (a == "--width")
            o.width = std::stoi(value());
        else if (a == "--height")
            o.height = std::stoi(value());
        else if (a == "--duration")
            o.duration = std::stod(value());
        else if (a == "--capture-at")
            o.captureAt = std::stod(value());
        else if (a == "--capture")
            o.capture = value();
        else if (a == "--csv")
            o.csv = value();
        else if (a == "--start-time")
            o.startTime = std::stof(value());
        else if (a == "--record")
            o.record = value();
        else if (a == "--ffmpeg")
            o.ffmpeg = value();
        else if (a == "--demo")
            o.demo = true;
        else if (a == "--launch")
            o.launch = true;
        else if (a == "--no-vsync")
            o.vsync = false;
        else if (a == "--help") {
            std::cout << "Baikonur: --demo --duration SECONDS --launch --start-time SECONDS "
                         "--width N --height N --no-vsync --csv FILE --capture-at SECONDS "
                         "--capture FILE.ppm --record FILE.mp4 --ffmpeg PATH\n";
            std::exit(0);
        } else
            throw std::runtime_error("Unknown option: " + a);
    }
    if (o.width < 640 || o.height < 360 || o.width > 3840 || o.height > 2160 || o.duration < 0)
        throw std::runtime_error("Invalid dimensions or duration");
    return o;
}
static float scrollDelta = 0;
int main(int argc, char **argv) {
    GLFWwindow *window = nullptr;
    bool imguiReady = false;
    try {
        Options o = options(argc, argv);
        glfwSetErrorCallback([](int, const char *msg) { std::cerr << "GLFW: " << msg << '\n'; });
        if (!glfwInit())
            throw std::runtime_error("GLFW initialization failed");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_SAMPLES, o.record.empty() ? 4 : 0);
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_FALSE);
        window = glfwCreateWindow(o.width, o.height, "BAIKONUR / Launch 09", nullptr, nullptr);
        if (!window)
            throw std::runtime_error("OpenGL 3.3 core context unavailable");
        glfwMakeContextCurrent(window);
        if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
            throw std::runtime_error("GLAD loading failed");
        glfwSwapInterval(o.vsync ? 1 : 0);
        std::cout << "GPU: " << glGetString(GL_RENDERER) << "\nOpenGL: " << glGetString(GL_VERSION)
                  << '\n';
        glfwSetScrollCallback(window,
                              [](GLFWwindow *, double, double y) { scrollDelta += float(y); });
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330 core");
        setupUI();
        imguiReady = true;
        {
            std::filesystem::path shaderDir =
                std::filesystem::absolute(argv[0]).parent_path() / "shaders";
            if (!std::filesystem::exists(shaderDir))
                shaderDir = "shaders";
            Scene scene(shaderDir.string());
            Particles particles(shaderDir.string());
            Camera camera;
            Mission mission;
            if (o.launch) {
                mission.launch();
                mission.clock = o.startTime;
                if (mission.clock >= 24)
                    mission.separate();
                camera.chase = true;
            }
            UIState ui;
            ui.gpu = (const char *)glGetString(GL_RENDERER);
            ui.demo = o.demo;
            std::ofstream csv;
            if (!o.csv.empty()) {
                csv.open(o.csv);
                csv << "wall_seconds,mission_seconds,altitude_m,particles,frame_ms\n";
            }
            std::unique_ptr<Recorder> recorder;
            std::unique_ptr<RenderTarget> target;
            int w, h;
            glfwGetFramebufferSize(window, &w, &h);
            if (!o.record.empty()) {
                target = std::make_unique<RenderTarget>(o.width, o.height);
                recorder = std::make_unique<Recorder>(o.ffmpeg, o.record, o.width, o.height);
            }
            double start = glfwGetTime(), last = start, fpsStart = start;
            int frames = 0;
            double mouseX = 0, mouseY = 0;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            std::array<bool, GLFW_KEY_LAST + 1> previous{};
            bool captured = false;
            std::vector<double> frameTimes;
            frameTimes.reserve(50000);
            int glErrors = 0;
            while (!glfwWindowShouldClose(window)) {
                double now = glfwGetTime(), wall = now - start, rawDt = now - last;
                last = now;
                float dt = float(std::min(rawDt, .1));
                if (o.duration > 0 && wall >= o.duration)
                    break;
                glfwPollEvents();
                glfwGetFramebufferSize(window, &w, &h);
                if (w == 0 || h == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    continue;
                }
                int windowWidth = w, windowHeight = h;
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                if (target) {
                    w = o.width;
                    h = o.height;
                    auto &io = ImGui::GetIO();
                    io.MousePos.x *= float(w) / io.DisplaySize.x;
                    io.MousePos.y *= float(h) / io.DisplaySize.y;
                    io.DisplaySize = {float(w), float(h)};
                    io.DisplayFramebufferScale = {1, 1};
                }
                ImGui::NewFrame();
                auto pressed = [&](int key) {
                    bool down = glfwGetKey(window, key) == GLFW_PRESS;
                    bool edge = down && !previous[key];
                    previous[key] = down;
                    return edge;
                };
                if (pressed(GLFW_KEY_ESCAPE))
                    glfwSetWindowShouldClose(window, true);
                if (!o.demo && !ImGui::GetIO().WantCaptureKeyboard) {
                    if (pressed(GLFW_KEY_SPACE))
                        mission.launch();
                    if (pressed(GLFW_KEY_P))
                        mission.paused = !mission.paused;
                    if (pressed(GLFW_KEY_R))
                        ui.reset = true;
                    if (pressed(GLFW_KEY_B))
                        mission.separate();
                    if (pressed(GLFW_KEY_C))
                        camera.chase = !camera.chase;
                    if (pressed(GLFW_KEY_K))
                        camera.shake = !camera.shake;
                    if (pressed(GLFW_KEY_L))
                        scene.lights = !scene.lights;
                    if (pressed(GLFW_KEY_N))
                        scene.night = !scene.night;
                    if (pressed(GLFW_KEY_F))
                        scene.wireframe = !scene.wireframe;
                    if (pressed(GLFW_KEY_H))
                        ui.help = !ui.help;
                    float dx = float((glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) -
                                     (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS));
                    float dy = float((glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) -
                                     (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS));
                    camera.orbit(dx * 120 * dt, dy * 120 * dt);
                    camera.zoom(float((glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) -
                                      (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)) *
                                12 * dt);
                }
                double mx, my;
                glfwGetCursorPos(window, &mx, &my);
                if (!o.demo && !ImGui::GetIO().WantCaptureMouse) {
                    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
                        camera.orbit(float(mx - mouseX), float(my - mouseY));
                    camera.zoom(scrollDelta);
                }
                scrollDelta = 0;
                mouseX = mx;
                mouseY = my;
                if (o.demo) {
                    directDemo(wall, mission, camera, scene, particles);
                    camera.yaw += dt * .10f;
                }
                if (ui.reset) {
                    mission.reset();
                    particles.reset();
                    camera = Camera{};
                    ui.reset = false;
                }
                mission.update(dt);
                particles.update(dt, mission);
                if (target)
                    target->bind();
                glViewport(0, 0, w, h);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                scene.render(mission, camera, w, h, float(now));
                particles.render(camera, w, h);
                frames++;
                if (now - fpsStart >= .5) {
                    ui.fps = float(frames / (now - fpsStart));
                    ui.frameMs = 1000.f / ui.fps;
                    frames = 0;
                    fpsStart = now;
                    std::string title = "BAIKONUR / " + mission.phase() + " / " +
                                        std::to_string(int(ui.fps)) + " FPS";
                    glfwSetWindowTitle(window, title.c_str());
                }
                ui.wall = wall;
                drawUI(mission, camera, scene, particles, ui);
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                if (pressed(GLFW_KEY_F12) ||
                    (!captured && o.captureAt >= 0 && wall >= o.captureAt)) {
                    screenshot(o.capture, w, h);
                    captured = true;
                    std::cout << "Screenshot: " << o.capture << '\n';
                }
                if (recorder)
                    recorder->frame(wall, w, h);
                if (target)
                    target->display(windowWidth, windowHeight);
                glfwSwapBuffers(window);
                if (wall > 2) {
                    frameTimes.push_back(rawDt * 1000);
                    if (csv)
                        csv << wall << ',' << mission.clock << ',' << mission.altitude() << ','
                            << particles.count() << ',' << rawDt * 1000 << '\n';
                }
                GLenum error;
                while ((error = glGetError()) != GL_NO_ERROR) {
                    std::cerr << "OpenGL error: " << error << '\n';
                    glErrors++;
                    if (glErrors > 10)
                        throw std::runtime_error("Repeated OpenGL errors");
                }
            }
            if (!frameTimes.empty()) {
                double sum = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0);
                std::sort(frameTimes.begin(), frameTimes.end());
                std::cout << "Frames: " << frameTimes.size()
                          << " Mean FPS: " << 1000 * frameTimes.size() / sum
                          << " p95 ms: " << frameTimes[size_t(frameTimes.size() * .95)]
                          << " GL errors: " << glErrors << '\n';
            }
            if (glErrors)
                throw std::runtime_error("OpenGL validation failed");
        }
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << '\n';
        if (imguiReady) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
        if (window)
            glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
}
