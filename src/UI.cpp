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
    s.Colors[ImGuiCol_TitleBg] = {.025f,.07f,.09f,.45f};
    s.Colors[ImGuiCol_TitleBgActive] = {.035f,.13f,.17f,.6f};
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
static void destinationUI(Mission &m, Camera &c, UIState &ui) {
    const auto size = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos({0,0});
    ImGui::SetNextWindowSize(size);
    ImGui::Begin("Choose destination", nullptr, ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings);
    ImGui::SetCursorPos({20,16});
    ImGui::TextUnformatted("CHOOSE YOUR DESTINATION");
    ImGui::SetCursorPos({20,42});
    ImGui::TextDisabled("Click a planet to travel. Moon, Mars and Mercury allow landing.");
    int chosen = -1;
    for (int i=0; i<destinationCount; ++i) {
        const auto &d = destinations[i];
        glm::vec4 clip = c.viewProjection * glm::vec4(d.center,1);
        if (clip.w <= 0) continue;
        glm::vec3 ndc = glm::vec3(clip)/clip.w;
        ImVec2 center((ndc.x*.5f+.5f)*size.x, (.5f-ndc.y*.5f)*size.y);
        glm::vec4 edge = c.viewProjection * glm::vec4(d.center + glm::vec3(std::max(d.radius,260.f),0,0),1);
        float radius = std::max(16.f, std::abs(edge.x/edge.w-ndc.x)*size.x*.5f);
        ImGui::PushID(i);
        ImGui::SetCursorPos({center.x-radius-8,center.y-radius-8});
        if (ImGui::InvisibleButton("planet",{2*radius+16,2*radius+16})) chosen=i;
        if (ImGui::IsItemHovered()) {
            ImGui::GetWindowDrawList()->AddCircle(center,radius+6,IM_COL32(70,230,255,255),48,2);
            ImGui::SetTooltip("%s",d.warning);
        }
        ImVec2 text = ImGui::CalcTextSize(d.name);
        ImGui::SetCursorPos({center.x-text.x*.5f-8,center.y+radius+9});
        if (ImGui::SmallButton(d.name)) chosen=i;
        ImGui::PopID();
    }
    ImGui::End();
    if (chosen >= 0) {
        ui.routeBlocked = !m.selectDestination(chosen);
        if (!ui.routeBlocked) {
            c.chase=true; c.distance=85; c.pitch=.25f;
            glm::vec3 approach=glm::normalize(m.route[1]-m.route[0]);
            c.yaw=std::atan2(-approach.x,-approach.z)+.4f;
        }
    }
}
static void gameplayUI(Mission &m, Camera &c, Scene &s, UIState &ui) {
    const auto size = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos({12,size.y-44}, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(.25f);
    ImGui::Begin("HUD toggle", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
    if (ImGui::SmallButton(ui.panel ? "Hide controls" : "Flight controls")) ui.panel=!ui.panel;
    ImGui::SameLine(); ImGui::TextDisabled("%.0f FPS",ui.fps);
    ImGui::End();
    if (!ui.panel) return;
    ImGui::SetNextWindowPos({12,12},ImGuiCond_Once);
    ImGui::SetNextWindowSize({std::min(340.f,size.x-24),0},ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(.48f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10,8));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6,4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6,4));
    if (ImGui::Begin("Baikonur | Flight controls", &ui.panel, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::SetWindowFontScale(.9f);
        if (m.flight==Flight::Choosing) {
            ImGui::TextColored({.2f,.8f,.95f,1},"KAZAKHSTAN / HOME");
            static int selected=0;
            ImGui::SetNextItemWidth(165);
            if (ImGui::BeginCombo("Route",destinations[selected].name)) {
                for (int i=0;i<destinationCount;++i)
                    if (ImGui::Selectable(destinations[i].name,selected==i)) selected=i;
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            ImGui::TextColored(destinations[selected].dangerous ? ImVec4(1,.7f,.25f,1) : ImVec4(.4f,.85f,.7f,1),
                               "%s risk",destinations[selected].dangerous ? "High" : "Low");
            if (ImGui::Button("Travel")) {
                if (m.selectDestination(selected)) {
                    c.chase=true; c.distance=85; c.pitch=.25f;
                    glm::vec3 approach=glm::normalize(m.route[1]-m.route[0]);
                    c.yaw=std::atan2(-approach.x,-approach.z)+.4f;
                    ui.routeBlocked=false;
                } else ui.routeBlocked=true;
            }
            ImGui::SameLine(); ImGui::TextDisabled("Choose your next stop");
            if (ui.routeBlocked) ImGui::TextWrapped("Route blocked. Choose another destination.");
        } else if (m.flight==Flight::Transit) {
            ImGui::Text("%s / %s",m.canLand() && m.progress()>.75f ? "Landing" : "En route",destinations[m.destination].name);
            ImGui::ProgressBar(m.progress(),{-1,5},"");
            ImGui::Text("Shield %.0f%%  |  %.0f%% complete",100*(1-m.danger()),100*m.progress());
            if (m.danger()>.35f) ImGui::TextColored({1,.65f,.25f,1},"Rising heat / unstable signal");
        } else if (m.flight==Flight::Arrived || m.flight==Flight::Destroyed) {
            if (!m.resultReady()) ImGui::TextDisabled("Receiving telemetry...");
            else {
                ImGui::TextColored(m.flight==Flight::Arrived ? ImVec4(.4f,.9f,.75f,1) : ImVec4(1,.6f,.3f,1),
                                   "%s",m.flight==Flight::Arrived ? (m.canLand() ? "Landed safely / crew safe" : "Stable orbit / crew safe") : "Signal lost / mission ended");
                if (ImGui::Button("Back to Kazakhstan")) ui.reset=true;
            }
        } else {
            ImGui::Text("%s  |  T %+.1f s",m.phase().c_str(),m.clock);
            if (ImGui::Button(m.running ? (m.paused ? "Resume [P]" : "Pause [P]") : "Launch [Space]")) {
                if (m.running) m.paused=!m.paused; else { m.launch(); c.chase=true; }
            }
        }
        if (m.inSpace()) {
            if (ImGui::SmallButton(m.paused ? "Resume [P]" : "Pause [P]")) m.paused=!m.paused;
        } else ImGui::SameLine();
        if (!m.inSpace()) { if (ImGui::SmallButton("Reset [R]")) ui.reset=true; }
        ImGui::SetNextItemWidth(230);
        ImGui::SliderFloat("Speed",&m.speed,.25f,20.f,"%.2fx",ImGuiSliderFlags_Logarithmic|ImGuiSliderFlags_AlwaysClamp);
        if (ImGui::CollapsingHeader("Camera & settings")) {
            ImGui::TextWrapped("Drag / WASD: look around. Scroll: zoom. Drag this panel by its title to move it.");
            ImGui::Checkbox("Chase camera [C]",&c.chase);
            ImGui::Checkbox("Shake [K]",&c.shake); ImGui::SameLine(); ImGui::Checkbox("Night [N]",&s.night);
            ImGui::Checkbox("Wireframe [F]",&s.wireframe);
            if (ImGui::Button("Restart mission")) ui.reset=true;
        }
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
}
void drawUI(Mission &m, Camera &c, Scene &s, const Particles &p, UIState &ui) {
    if (!ui.demo) {
        if (m.flight == Flight::Choosing) destinationUI(m,c,ui);
        else gameplayUI(m, c, s, ui);
        return;
    }
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
        ImGui::SliderFloat("Speed", &m.speed, .25f, 20.f, "%.2fx",
                           ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp);
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
