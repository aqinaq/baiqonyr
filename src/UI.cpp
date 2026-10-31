#include "UI.hpp"
#include <cstdio>
#include <imgui.h>
#include <stdexcept>
namespace bk {
static ImFont *titleFont = nullptr, *metricFont = nullptr, *captionFont = nullptr;
void setupUI(const std::string &fontPath) {
    auto &io = ImGui::GetIO();
    // TrueType glyphs baked at 3x support Retina windows and native 4K capture.
    ImFontConfig config;
    config.OversampleH = 2;
    config.OversampleV = 2;
    io.FontDefault = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 45.f, &config);
    titleFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 84.f, &config);
    metricFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 66.f, &config);
    captionFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 54.f, &config);
    if (!io.FontDefault || !titleFont || !metricFont || !captionFont)
        throw std::runtime_error("Bundled Roboto font missing; keep assets beside the executable");
    io.FontGlobalScale = 1.f / 3.f;
    ImGui::StyleColorsDark();
    auto &s = ImGui::GetStyle();
    s.WindowRounding = 8;
    s.FrameRounding = 4;
    s.WindowPadding = {18, 16};
    s.ItemSpacing = {9, 7};
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
            c.distance = 110;
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
        // Include falling boosters below the core, clear of the caption overlay.
        c.distance = std::min(110.f, 55.f + (m.clock - 22) * 9.f);
        c.pitch = .12f - .47f * std::clamp((m.clock - 22) / 2.f, 0.f, 1.f);
    }
    if (t >= 198)
        m.paused = true;
    // Main applies frame-independent orbit using elapsed wall time.
}
static void spaceUI(Mission &m, Camera &c, UIState &ui) {
    auto size = ImGui::GetIO().DisplaySize;
    auto *background = ImGui::GetBackgroundDrawList();
    background->AddRectFilled({0,0}, {size.x,22}, IM_COL32(0,0,0,255));
    background->AddRectFilled({0,size.y-22}, size, IM_COL32(0,0,0,255));
    ImGui::SetNextWindowPos({20,32});
    ImGui::SetNextWindowSize({std::min(370.f, size.x-40),0});
    ImGui::Begin("Flight telemetry", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextColored({.55f,.8f,.9f,1}, "BAIKONUR / BEYOND THE BLUE");
    ImGui::TextWrapped("%s", m.phase().c_str());
    if (m.destination >= 0) ImGui::Text("DESTINATION  /  %s", destinations[m.destination].name);
    if (m.flight == Flight::Transit) {
        ImGui::ProgressBar(m.progress(), {-1,5}, "");
        ImGui::Text("HULL %03.0f%%   /   SIGNAL %03.0f%%", 100*(1-m.danger()),100*(1-m.danger()*.95f));
        if (m.danger() > 0) ImGui::TextWrapped("%s", destinations[m.destination].warning);
        if (m.danger() > .65f) ImGui::TextColored({1,.3f,.18f,1}, "CRITICAL / STRUCTURAL INTEGRITY LOST");
    }
    if (ImGui::Button("RESTART [R]")) ui.reset = true;
    ImGui::SameLine();
    if (ImGui::Button(m.paused ? "RESUME [P]" : "PAUSE [P]")) m.paused = !m.paused;
    ImGui::TextDisabled("Drag / WASD: look around  |  Scroll: zoom");
    ImGui::End();
    if (m.flight == Flight::Choosing || m.flight == Flight::Arrived ||
        (m.flight == Flight::Destroyed && m.aftermath > 2.f)) {
        ImGui::SetNextWindowPos({size.x*.5f, size.y*.55f}, ImGuiCond_Always, {.5f,.5f});
        ImGui::SetNextWindowSize({std::min(520.f,size.x-32), std::min(450.f,size.y-90)});
        ImGui::Begin("Navigation", nullptr, ImGuiWindowFlags_NoDecoration);
        ImGui::PushFont(titleFont);
        ImGui::TextUnformatted(m.flight == Flight::Choosing ? "WHERE DO WE GO?" :
                              m.flight == Flight::Arrived ? "ORBIT ACHIEVED" : "GAME OVER");
        ImGui::PopFont();
        if (m.flight == Flight::Choosing) {
            ImGui::TextWrapped("Earth is behind us. Choose a destination to begin the transfer.");
            ImGui::TextDisabled("Cinematic scale / compressed travel time");
            ImGui::Separator();
            for (int i = 0; i < destinationCount; ++i) {
                const auto &d = destinations[i];
                ImGui::PushID(i);
                if (ImGui::Button(d.name, {110,30})) {
                    m.selectDestination(i);
                    c.chase = true;
                    glm::vec3 approach = glm::normalize(d.center-m.position());
                    c.yaw = std::atan2(-approach.x,-approach.z)+.3f;
                    c.pitch = .12f;
                    c.distance = 58;
                }
                ImGui::SameLine();
                ImGui::TextColored(d.dangerous ? ImVec4(1,.4f,.23f,1) : ImVec4(.5f,.8f,.75f,1),
                                   "%s", d.dangerous ? "FATAL ENTRY" : "SAFE ORBIT");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", d.warning);
                ImGui::PopID();
            }
        } else {
            ImGui::Spacing();
            if (m.flight == Flight::Destroyed) {
                ImGui::TextColored({1,.35f,.2f,1}, "VEHICLE DESTROYED / CREW LOST");
                ImGui::TextWrapped("%s", destinations[m.destination].warning);
                ImGui::TextWrapped("No response from the spacecraft. Mission control has closed the channel.");
            } else {
                ImGui::TextWrapped("%s survey complete. Crew safe. Engines shut down.", destinations[m.destination].name);
                ImGui::TextWrapped("Take a moment. There is a whole universe outside the window.");
            }
            ImGui::Spacing();
            ImGui::TextWrapped("Fictional mission inspired by spaceflight hazards. Distances and outcomes are simplified for gameplay.");
            if (ImGui::Button("RETURN TO BAIKONUR", {-1,40})) ui.reset = true;
        }
        ImGui::End();
    }
}
void drawUI(Mission &m, Camera &c, Scene &s, const Particles &p, UIState &ui) {
    if (m.inSpace() && !ui.demo) { spaceUI(m, c, ui); return; }
    const auto &io = ImGui::GetIO();
    ImGui::SetNextWindowPos({24, 24});
    ImGui::SetNextWindowSize({294, 0});
    ImGui::Begin("Mission", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextColored({.45f, .86f, .8f, 1}, "BAIKONUR / FLIGHT CONTROL");
    ImGui::Spacing();
    ImGui::PushFont(titleFont);
    ImGui::TextUnformatted("Launch 09");
    ImGui::PopFont();
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
    if (!ui.demo) {
        if (ImGui::Button(m.running ? "PAUSE / RESUME  [P]" : "START COUNTDOWN  [SPACE]",
                          {-1, 36})) {
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
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 60.f);
        ImGui::SliderFloat("Speed", &m.speed, .25f, 2.f, "%.2fx");
    } else {
        ImGui::Spacing();
        ImGui::Text("Camera: %s", c.chase ? "Chase" : "Orbit");
        ImGui::Text("Light: %s / %s", s.night ? "Night" : "Day",
                    s.lights ? "Gantry on" : "Gantry off");
        ImGui::Text("Mission: %s", m.paused ? "Paused" : m.running ? "Running" : "Ready");
    }
    ImGui::TextDisabled("BOOSTERS  %s", m.separated ? "DETACHED" : "ATTACHED");
    ImGui::End();
    ImGui::SetNextWindowPos({io.DisplaySize.x - 292, 24});
    ImGui::SetNextWindowSize({268, 0});
    ImGui::Begin("Performance", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextColored({.45f, .86f, .8f, 1}, "LIVE RENDERER");
    ImGui::PushFont(metricFont);
    ImGui::Text("%5.1f FPS", ui.fps);
    ImGui::PopFont();
    ImGui::Text("%.2f ms / frame", ui.frameMs);
    ImGui::Text("%d x %d  /  4x MSAA", ui.pixelWidth, ui.pixelHeight);
    ImGui::Text("OpenGL core / indexed meshes");
    ImGui::TextWrapped("%s", ui.gpu.c_str());
    ImGui::End();
    if (ui.help) {
        ImGui::SetNextWindowPos({io.DisplaySize.x - 534, io.DisplaySize.y - (ui.demo ? 178 : 104)});
        ImGui::SetNextWindowSize({510, 88});
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
        ImGui::SetNextWindowPos({io.DisplaySize.x * .5f - 480, io.DisplaySize.y - 90});
        ImGui::SetNextWindowSize({960, 70});
        ImGui::Begin("Demo captions", nullptr, ImGuiWindowFlags_NoDecoration);
        ImGui::PushFont(captionFont);
        ImGui::TextWrapped("%s", caption);
        ImGui::PopFont();
        ImGui::End();
    }
}
} // namespace bk
