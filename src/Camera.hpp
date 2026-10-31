#pragma once
#include "Simulation.hpp"
#include <glm/gtc/matrix_transform.hpp>
namespace bk {
class Camera {
  public:
    float yaw = .8f, pitch = .2f, distance = 44.f;
    bool chase = false, shake = true;
    glm::vec3 eye{0}, target{0, 9, 0};
    void orbit(float x, float y) {
        yaw -= x * .006f;
        pitch = std::clamp(pitch + y * .006f, -.12f, 1.45f);
    }
    void zoom(float amount) { distance = std::clamp(distance * std::exp(-amount * .08f), 12.f, 12000.f); }
    glm::mat4 view(const Mission &m, float realTime) {
        target = glm::vec3(0, 9, 0) + ((chase || m.inSpace()) ? m.position() : glm::vec3(0));
        if (m.flight == Flight::Choosing) target = {500, 0, 0};
        eye = target + distance * glm::vec3(std::sin(yaw) * std::cos(pitch), std::sin(pitch),
                                            std::cos(yaw) * std::cos(pitch));
        if (shake && m.ignition() && m.clock < 4 && !m.paused) {
            float amp = .14f * (1 - std::clamp((m.clock + 2) / 6.f, 0.f, 1.f));
            eye += amp * glm::vec3(std::sin(realTime * 83), std::cos(realTime * 97),
                                   std::sin(realTime * 71));
        }
        if (shake && !m.paused && m.flight == Flight::Transit && m.danger() > 0)
            eye += m.danger()*.22f*glm::vec3(std::sin(realTime*53),std::cos(realTime*47),0);
        return glm::lookAt(eye, target, glm::vec3(0, 1, 0));
    }
};
} // namespace bk
