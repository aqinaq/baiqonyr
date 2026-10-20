#include "UI.hpp"
#include <cstdio>
#include <imgui.h>
namespace bk {
void setupUI() {
    ImGui::StyleColorsDark();
    auto &s = ImGui::GetStyle();
    s.WindowRounding = 8;
    s.FrameRounding = 4;
    s.WindowPadding = {18, 16};
    s.ItemSpacing = {9, 10};
    s.FramePadding = {9, 7};
    s.WindowBorderSize = 1;
    s.Colors[ImGuiCol_WindowBg] = {.025f, .045f, .065f, .94f};
    s.Colors[ImGuiCol_Border] = {.15f, .23f, .28f, .7f};
    s.Colors[ImGuiCol_Text] = {.86f, .9f, .9f, 1};
    s.Colors[ImGuiCol_Button] = {.08f, .31f, .34f, 1};
    s.Colors[ImGuiCol_ButtonHovered] = {.12f, .44f, .46f, 1};
    s.Colors[ImGuiCol_CheckMark] = {.4f, .85f, .78f, 1};
    s.Colors[ImGuiCol_SliderGrab] = {.4f, .85f, .78f, 1};
    ImGui::GetIO().IniFilename = nullptr;
}
void directDemo(double t, Mission &m, Camera &c, Scene &s, Particles &p) {
    // Timed input choreography; rendering and simulation still use real wall-clock dt.
    static int previous = -1;
    int stage = t < 25 ? 0 : t < 45 ? 1 : t < 65 ? 2 : t < 100 ? 3 : t < 130 ? 4 : t < 164 ? 5 : 6;
    if (stage != previous) {
        if (stage == 0 || stage == 5) {
            m.reset();
            p.reset();
            c.chase = false;
            c.distance = 44;
            c.pitch = .2f;
            s.night = false;
            s.wireframe = false;
        }
        if (stage == 1) {
            s.night = true;
            s.lights = true;
        }
        if (stage == 2) {
            s.night = false;
            s.wireframe = true;
        }
        if (stage == 3) {
            s.wireframe = false;
            m.reset();
            p.reset();
            m.launch();
            c.chase = true;
            c.distance = 42;
            c.pitch = .12f;
        }
        if (stage == 4) {
            m.paused = true;
            c.distance = 68;
            c.pitch = -.35f;
        }
        if (stage == 6) {
            m.reset();
            p.reset();
            m.launch();
            s.night = true;
            c.chase = true;
            c.distance = 43;
            c.pitch = .12f;
        }
        previous = stage;
    }
    if (stage == 1)
        s.lights = std::fmod(t, 8) > 4;
    if (stage == 2)
        s.wireframe = t < 55;
    if ((stage == 3 || stage == 6) && m.clock > 22) {
        c.distance = std::min(78.f, 43.f + (m.clock - 22) * 4.f);
        c.pitch = .12f - .47f * std::clamp((m.clock-22)/2.f,0.f,1.f);
    }
    if (t >= 198)
        m.paused = true;
    // Main applies frame-independent orbit using elapsed wall time.
}
void drawUI(Mission &m, Camera &c, Scene &s, const Particles &p, UIState &ui) {
    const auto &io = ImGui::GetIO();
    ImGui::SetNextWindowPos({24, 24});
    ImGui::SetNextWindowSize({294, 0});
    ImGui::Begin("Mission", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextColored({.45f, .86f, .8f, 1}, "BAIKONUR / FLIGHT CONTROL");
    ImGui::Spacing();
    ImGui::SetWindowFontScale(1.8f);
    ImGui::TextUnformatted("Launch 09");
    ImGui::SetWindowFontScale(1);
    ImGui::TextDisabled("A COMPUTER GRAPHICS STUDY");
    ImGui::Separator();
    ImGui::TextColored({1, .66f, .32f, 1}, "%s", m.phase().c_str());
    ImGui::Text("T %c %06.1f s", m.clock < 0 ? '-' : '+', std::abs(m.clock));
    ImGui::Spacing();
    ImGui::Text("ALTITUDE       %8.1f m", m.altitude());
    ImGui::Text("VERTICAL SPEED %8.1f m/s", m.velocity());
    ImGui::Text("PARTICLES      %8zu", p.count());
    ImGui::TextDisabled("Compressed visual trajectory");
    ImGui::Separator();
    ImGui::BeginDisabled(ui.demo);
    if (ImGui::Button(m.running ? "PAUSE / RESUME  [P]" : "START COUNTDOWN  [SPACE]", {-1, 36})) {
        if (m.running)
            m.paused = !m.paused;
        else
            m.launch();
    }
    if (ImGui::Button("RESET  [R]", {123, 0}))
        ui.reset = true;
    ImGui::SameLine();
    if (ImGui::Button("SEPARATE [B]", {123, 0}))
        m.separate();
    ImGui::Checkbox("Chase camera [C]", &c.chase);
    ImGui::Checkbox("Ignition shake [K]", &c.shake);
    ImGui::Checkbox("Gantry light [L]", &s.lights);
    ImGui::Checkbox("Night lighting [N]", &s.night);
    ImGui::Checkbox("Wireframe [F]", &s.wireframe);
    ImGui::SliderFloat("Speed", &m.speed, .25f, 2.f, "%.2fx");
    ImGui::EndDisabled();
    ImGui::TextDisabled("BOOSTERS  %s", m.separated ? "DETACHED" : "ATTACHED");
    ImGui::End();
    ImGui::SetNextWindowPos({io.DisplaySize.x - 292, 24});
    ImGui::SetNextWindowSize({268, 0});
    ImGui::Begin("Performance", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextColored({.45f, .86f, .8f, 1}, "LIVE RENDERER");
    ImGui::SetWindowFontScale(1.4f);
    ImGui::Text("%5.1f FPS", ui.fps);
    ImGui::SetWindowFontScale(1);
    ImGui::Text("%.2f ms / frame", ui.frameMs);
    ImGui::Text("%.0f x %.0f", io.DisplaySize.x, io.DisplaySize.y);
    ImGui::Text("OpenGL core / indexed meshes");
    ImGui::TextWrapped("%s", ui.gpu.c_str());
    ImGui::End();
    if (ui.help) {
        ImGui::SetNextWindowPos({io.DisplaySize.x - 534, io.DisplaySize.y - 100});
        ImGui::SetNextWindowSize({510, 76});
        ImGui::Begin("Help", nullptr, ImGuiWindowFlags_NoDecoration);
        ImGui::TextColored({.45f, .86f, .8f, 1}, "CAMERA & CONTROLS");
        ImGui::Text("Drag: orbit   Scroll: zoom   WASD: orbit / pitch");
        ImGui::Text("Q/E: zoom   H: help   F12: screenshot   ESC: exit");
        ImGui::End();
    }
    if (ui.demo) {
        const char *caption =
            ui.wall < 25 ? "R2 / R3  |  A launch complex built from indexed primitives and a "
                           "rocket hierarchy"
            : ui.wall < 45
                ? "R4  |  Per-fragment Blinn-Phong shading. Toggle the blue gantry light."
            : ui.wall < 55
                ? "R1  |  OpenGL core pipeline: triangles, VAO / VBO / EBO, original GLSL shaders"
            : ui.wall < 65
                ? "R5 / R6  |  UV-mapped concrete, mipmaps, procedural steppe, orbit controls"
            : ui.wall < 83
                ? "CORE  |  Countdown, ignition, fire and smoke. Camera shake follows ignition."
            : ui.wall < 99
                ? "CORE  |  Chase camera tracks ascent. The atmosphere darkens with altitude."
            : ui.wall < 100 ? "STRETCH  |  Four boosters detach in world space and fall away."
            : ui.wall < 130 ? "STRETCH  |  Stars emerge at altitude. Pause freezes the mission; "
                              "the camera stays free."
            : ui.wall < 164
                ? "R6 / R7  |  Reset, change lighting, replay. Live FPS stays visible throughout."
            : ui.wall < 196
                ? "NIGHT LAUNCH  |  Moving engine light, instanced flame, sorted translucent smoke"
                : "BAIKONUR  |  Booster separation against the stars";
        ImGui::SetNextWindowPos({io.DisplaySize.x * .5f - 480, io.DisplaySize.y - 190});
        ImGui::SetNextWindowSize({960, 65});
        ImGui::Begin("Demo captions", nullptr, ImGuiWindowFlags_NoDecoration);
        ImGui::TextWrapped("%s", caption);
        ImGui::End();
    }
}
} // namespace bk
