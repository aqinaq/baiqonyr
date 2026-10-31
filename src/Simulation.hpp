#pragma once
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
namespace bk {
constexpr float pi = 3.14159265358979323846f;
enum class Flight { Ascent, Choosing, Transit, Arrived, Destroyed };
struct Destination {
    const char *name;
    glm::vec3 center, color;
    float radius;
    const char *warning;
    bool dangerous;
};
inline const Destination destinations[] = {
    {"MOON", {-170, 970, -350}, {.65,.68,.72}, 55, "Stable lunar orbit. No landing required.", false},
    {"MARS", {240, 1040, -640}, {.72,.25,.12}, 85, "Safe orbital survey above the red planet.", false},
    {"SUN", {-720, 1200, -1250}, {1,.55,.12}, 220, "FATAL: thermal overload. Heat shield cannot survive.", true},
    {"VENUS", {520, 850, -900}, {.86,.65,.34}, 105, "FATAL: atmospheric entry overwhelms hull and cooling.", true},
    {"JUPITER", {-900, 1100, 250}, {.74,.57,.39}, 230, "FATAL: deep entry. Pressure crushes the spacecraft.", true},
    {"SATURN", {950, 1150, 100}, {.82,.72,.48}, 175, "Safe observation orbit outside the rings.", false},
    {"MERCURY", {-420, 850, -850}, {.45,.42,.38}, 38, "Safe orbital survey. Keep the sun shield aligned.", false},
    {"URANUS", {500, 1400, 1000}, {.32,.72,.77}, 105, "Safe distant observation orbit.", false},
    {"NEPTUNE", {-550, 1300, 1150}, {.15,.32,.8}, 110, "Safe distant observation orbit.", false}
};
constexpr int destinationCount = sizeof(destinations) / sizeof(destinations[0]);
// Simulation metres and seconds are deliberately compressed for a classroom demo.
struct Mission {
    bool running = false, paused = false, separated = false;
    float clock = -8.f, speed = 1.f, separationTime = 24.f;
    Flight flight = Flight::Ascent;
    int destination = -1;
    float travel = 0, aftermath = 0;
    static constexpr float orbitTime = 40.f, travelDuration = 18.f;
    bool inSpace() const { return flight != Flight::Ascent; }
    bool selectDestination(int index) {
        if (flight != Flight::Choosing || index < 0 || index >= destinationCount) return false;
        destination = index;
        travel = 0;
        flight = Flight::Transit;
        return true;
    }
    float progress() const { return std::clamp(travel / travelDuration, 0.f, 1.f); }
    float danger() const {
        return destination >= 0 && destinations[destination].dangerous
            ? std::clamp((progress() - .45f) / .55f, 0.f, 1.f) : 0.f;
    }
    glm::vec3 position() const {
        glm::vec3 origin(0, altitudeAt(std::min(clock, orbitTime)), 0);
        if (destination < 0) return origin;
        const auto &d = destinations[destination];
        glm::vec3 end = d.center + glm::normalize(origin - d.center) * (d.radius + 38.f);
        float t = progress();
        return glm::mix(origin, end, t*t*(3-2*t));
    }
    void launch() {
        if (!running) {
            running = true;
            clock = -8.f;
        }
    }
    void reset() { *this = Mission{}; }
    void update(float dt) {
        if (!running || paused) return;
        float step = std::max(0.f, dt) * speed;
        if (flight == Flight::Destroyed) { aftermath = std::min(aftermath + step, 12.f); return; }
        if (flight == Flight::Choosing || flight == Flight::Arrived) return;
        if (flight == Flight::Transit) {
            travel = std::min(travel + step, travelDuration);
            if (travel >= travelDuration)
                flight = destinations[destination].dangerous ? Flight::Destroyed : Flight::Arrived;
            return;
        }
        clock = std::min(clock + step, orbitTime);
        if (clock >= 24.f) separate();
        if (clock >= orbitTime) flight = Flight::Choosing;
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
    float altitude() const { return position().y; }
    float velocity() const { return inSpace() ? 0.f : 1.1f * std::max(0.f, clock); }
    bool ignition() const { return running && clock >= -2.f && (flight == Flight::Ascent || flight == Flight::Transit); }
    glm::mat4 rocket() const { auto transform = glm::translate(glm::mat4(1), position());
        if (destination >= 0) {
            glm::vec3 direction = glm::normalize(destinations[destination].center - glm::vec3(0, altitudeAt(orbitTime), 0));
            glm::vec3 axis = glm::normalize(glm::cross(glm::vec3(0,1,0), direction));
            transform = glm::rotate(transform, std::acos(direction.y) * std::min(progress()*5.f, 1.f), axis);
        }
        return transform; }
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
        if (flight == Flight::Choosing) return "AWAITING DESTINATION";
        if (flight == Flight::Transit) return danger() > .4f ? "WARNING / HULL FAILURE" : "INTERPLANETARY TRANSFER";
        if (flight == Flight::Arrived) return "ORBIT ACHIEVED";
        if (flight == Flight::Destroyed) return "SIGNAL LOST / GAME OVER";
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
