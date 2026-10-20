#pragma once
#include "Particles.hpp"
#include "Scene.hpp"
namespace bk {
struct UIState {
    bool help = true, reset = false;
    float fps = 0, frameMs = 0;
    double wall = 0;
    bool demo = false;
    std::string gpu;
};
void setupUI();
void drawUI(Mission &, Camera &, Scene &, const Particles &, UIState &);
void directDemo(double time, Mission &, Camera &, Scene &, Particles &);
} // namespace bk
