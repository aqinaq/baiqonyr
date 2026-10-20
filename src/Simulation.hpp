#pragma once
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
namespace bk {
constexpr float pi = 3.14159265358979323846f;
// Simulation metres and seconds are deliberately compressed for a classroom demo.
struct Mission {
    bool running = false, paused = false, separated = false;
    float clock = -8.f, speed = 1.f, separationTime = 24.f;
    void launch() {
        if (!running) {
            running = true;
            clock = -8.f;
        }
    }
    void reset() { *this = Mission{}; }
    void update(float dt) {
        if (running && !paused) {
            clock += std::max(0.f, dt) * speed;
            if (clock >= 24.f)
                separate();
        }
    }
    void separate() {
        if (clock >= 5.f && !separated) {
            separated = true;
            separationTime = clock;
        }
    }
    float altitudeAt(float t) const {
        t = std::max(0.f, t);
        return .55f * t * t;
    }
    float altitude() const { return altitudeAt(clock); }
    float velocity() const { return 1.1f * std::max(0.f, clock); }
    bool ignition() const { return running && clock >= -2.f; }
    glm::mat4 rocket() const { return glm::translate(glm::mat4(1), glm::vec3(0, altitude(), 0)); }
    glm::mat4 booster(int i) const {
        float a = float(i) * pi * .5f;
        glm::vec3 radial(std::cos(a), 0, std::sin(a));
        auto local = glm::translate(glm::mat4(1), radial * 1.72f + glm::vec3(0, 3.8f, 0));
        if (!separated)
            return rocket() * local;
        float t = std::max(0.f, clock - separationTime);
        // Detach in world space, preserve position and initial vertical velocity.
        glm::vec3 p =
            radial * (1.72f + 1.8f * t) +
            glm::vec3(
                0, 3.8f + altitudeAt(separationTime) + 1.1f * separationTime * t - 1.8f * t * t, 0);
        p.y = std::max(.8f, p.y);
        return glm::rotate(glm::translate(glm::mat4(1), p), t * .35f,
                           glm::vec3(radial.z, 0, -radial.x));
    }
    std::string phase() const {
        if (!running)
            return "READY FOR LAUNCH";
        if (clock < -2)
            return "COUNTDOWN";
        if (clock < 0)
            return "ENGINE IGNITION";
        if (separated)
            return "CORE STAGE ASCENT";
        return "POWERED ASCENT";
    }
};
inline float atmosphere(float altitude) {
    return std::clamp(altitude / 450.f, 0.f, 1.f);
}
} // namespace bk
