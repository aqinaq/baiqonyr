#include "Particles.hpp"
#include <cstddef>
namespace bk {
Particles::Particles(const std::string &path)
    : shader(path + "/particle.vert", path + "/particle.frag") {
    pool.reserve(3600);
    upload.reserve(3600);
    float vertices[] = {-1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1};
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &quad);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glGenBuffers(1, &instances);
    glBindBuffer(GL_ARRAY_BUFFER, instances);
    glBufferData(GL_ARRAY_BUFFER, 3600 * sizeof(ParticleGPU), nullptr, GL_STREAM_DRAW);
    for (int i = 0; i < 2; i++) {
        glEnableVertexAttribArray(i + 1);
        glVertexAttribPointer(i + 1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleGPU),
                              (void *)(size_t(i) * sizeof(glm::vec4)));
        glVertexAttribDivisor(i + 1, 1);
    }
}
Particles::~Particles() {
    glDeleteBuffers(1, &quad);
    glDeleteBuffers(1, &instances);
    glDeleteVertexArrays(1, &vao);
}
float Particles::random() {
    return float(rng() % 10000) / 10000.f;
}
void Particles::emit(const Mission &m, bool fire) {
    if (pool.size() >= 3600)
        return;
    float a = random() * 2 * pi, r = random() * (fire ? .48f : 1.8f), alt = m.altitude();
    glm::vec3 p(r * std::cos(a), alt + 1.2f, r * std::sin(a));
    glm::vec3 v((random() - .5f) * 3, -8 - random() * 7, (random() - .5f) * 3);
    if (!fire) {
        p.y = std::max(.6f, alt - .4f);
        v = {std::cos(a) * (3 + random() * 4), 1 + random() * 2, std::sin(a) * (3 + random() * 4)};
    }
    pool.push_back({p, v, 0, fire ? .35f + random() * .65f : 3 + random() * 3,
                    fire ? .25f + random() * .3f : .7f + random(), fire});
}
void Particles::update(float dt, const Mission &m) {
    if (m.paused)
        return;
    dt *= m.speed;
    for (auto &p : pool) {
        p.age += dt;
        p.p += p.v * dt;
        if (!p.fire) {
            p.v.y += .2f * dt;
            p.size += dt * .65f;
        } else
            p.size += dt * .45f;
        p.p.y = std::max(.25f, p.p.y);
    }
    pool.erase(std::remove_if(pool.begin(), pool.end(), [](auto &p) { return p.age >= p.life; }),
               pool.end());
    if (m.ignition()) {
        spawn += dt * 700;
        int n = std::min(int(spawn), 600);
        spawn -= n;
        for (int i = 0; i < n; i++)
            emit(m, (emitted++ % 3) != 0);
    }
}
void Particles::render(const Camera &c, int width, int height) {
    glm::mat4 view = glm::lookAt(c.eye, c.target, glm::vec3(0, 1, 0));
    shader.use();
    shader.set("view", view);
    shader.set("projection",
               glm::perspective(glm::radians(48.f), float(width) / height, .1f, 20000.f));
    // Alpha smoke is sorted back to front. Additive fire does not need sorting.
    std::sort(pool.begin(), pool.end(), [&](const Particle &a, const Particle &b) {
        return glm::dot(a.p - c.eye, a.p - c.eye) > glm::dot(b.p - c.eye, b.p - c.eye);
    });
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBindVertexArray(vao);
    for (bool fire : {false, true}) {
        upload.clear();
        for (const auto &p : pool)
            if (p.fire == fire) {
                float t = p.age / p.life;
                glm::vec4 col;
                if (fire)
                    col = {1.f, .62f * (1 - t) + .05f, .08f * (1 - t), .82f * (1 - t)};
                else
                    col = {.42f + .18f * t, .43f + .15f * t, .44f + .13f * t,
                           .26f * std::sin(pi * t)};
                upload.push_back({glm::vec4(p.p, p.size), col});
            }
        glBlendFunc(GL_SRC_ALPHA, fire ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);
        glBindBuffer(GL_ARRAY_BUFFER, instances);
        glBufferSubData(GL_ARRAY_BUFFER, 0, upload.size() * sizeof(ParticleGPU), upload.data());
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)upload.size());
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
} // namespace bk
