#pragma once
#include "Camera.hpp"
#include "Shader.hpp"
#include <random>
#include <vector>
namespace bk {
struct Particle {
    glm::vec3 p, v;
    float age, life, size;
    bool fire;
};
struct ParticleGPU {
    glm::vec4 positionSize, color;
};
class Particles {
    Shader shader;
    GLuint vao = 0, quad = 0, instances = 0;
    std::vector<Particle> pool;
    std::vector<ParticleGPU> upload;
    std::mt19937 rng{904};
    float spawn = 0;
    unsigned emitted = 0;
    float random();
    void emit(const Mission &m, bool fire);

  public:
    explicit Particles(const std::string &path);
    ~Particles();
    void reset() {
        pool.clear();
        spawn = 0;
        emitted = 0;
        rng.seed(904);
    }
    void update(float dt, const Mission &m);
    void render(const Camera &c, int width, int height);
    size_t count() const { return pool.size(); }
};
} // namespace bk
