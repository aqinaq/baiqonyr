#include "Mesh.hpp"
#include "Simulation.hpp"
#include <cstddef>
#include <set>
#include <utility>
namespace bk {
Geometry cube() {
    Geometry g;
    const glm::vec3 normals[] = {{1, 0, 0},  {-1, 0, 0}, {0, 1, 0},
                                 {0, -1, 0}, {0, 0, 1},  {0, 0, -1}};
    for (auto n : normals) {
        glm::vec3 u = std::abs(n.y) > .5f ? glm::vec3(1, 0, 0)
                                          : glm::normalize(glm::cross(glm::vec3(0, 1, 0), n));
        glm::vec3 v = glm::cross(n, u);
        unsigned b = (unsigned)g.vertices.size();
        g.vertices.push_back({(n - u - v) * .5f, n, {0, 0}});
        g.vertices.push_back({(n + u - v) * .5f, n, {1, 0}});
        g.vertices.push_back({(n + u + v) * .5f, n, {1, 1}});
        g.vertices.push_back({(n - u + v) * .5f, n, {0, 1}});
        for (unsigned j : {0u, 1u, 2u, 0u, 2u, 3u})
            g.indices.push_back(b + j);
    }
    return g;
}
Geometry cylinder(float top, int segments) {
    Geometry g;
    for (int i = 0; i <= segments; i++) {
        float a = 2 * pi * i / segments;
        glm::vec3 n = glm::normalize(glm::vec3(std::cos(a), 1 - top, std::sin(a)));
        g.vertices.push_back({{std::cos(a), -.5f, std::sin(a)}, n, {float(i) / segments, 0}});
        g.vertices.push_back(
            {{top * std::cos(a), .5f, top * std::sin(a)}, n, {float(i) / segments, 1}});
    }
    for (int i = 0; i < segments; i++) {
        unsigned b = 2 * i;
        for (unsigned j : {b, b + 1, b + 2, b + 1, b + 3, b + 2})
            g.indices.push_back(j);
    }
    for (int side = 0; side < 2; side++) {
        float y = side ? .5f : -.5f, r = side ? top : 1.f;
        glm::vec3 n(0, side ? 1.f : -1.f, 0);
        unsigned center = (unsigned)g.vertices.size();
        g.vertices.push_back({{0, y, 0}, n, {.5f, .5f}});
        for (int i = 0; i <= segments; i++) {
            float a = 2 * pi * i / segments;
            g.vertices.push_back({{r * std::cos(a), y, r * std::sin(a)},
                                  n,
                                  {.5f + .5f * std::cos(a), .5f + .5f * std::sin(a)}});
        }
        for (int i = 0; i < segments; i++) {
            unsigned a = center + 1 + i, b = a + 1;
            g.indices.insert(g.indices.end(), side ? std::initializer_list<unsigned>{center, b, a}
                                                   : std::initializer_list<unsigned>{center, a, b});
        }
    }
    return g;
}
Geometry sphere(int rings, int segments) {
    Geometry g;
    for (int r = 0; r <= rings; r++)
        for (int s = 0; s <= segments; s++) {
            float a = pi * r / rings, b = 2 * pi * s / segments;
            glm::vec3 p(std::sin(a) * std::cos(b), std::cos(a), std::sin(a) * std::sin(b));
            g.vertices.push_back({p, p, {float(s) / segments, float(r) / rings}});
        }
    for (int r = 0; r < rings; r++)
        for (int s = 0; s < segments; s++) {
            unsigned a = r * (segments + 1) + s, b = a + segments + 1;
            for (unsigned j : {a, a + 1, b, a + 1, b + 1, b})
                g.indices.push_back(j);
        }
    return g;
}
Mesh::Mesh(const Geometry &g) : count((GLsizei)g.indices.size()) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, g.vertices.size() * sizeof(Vertex), g.vertices.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, g.indices.size() * sizeof(unsigned), g.indices.data(),
                 GL_STATIC_DRAW);
    // Reuse the exact triangle topology; indexed lines are faster on the tested GPU.
    std::set<std::pair<unsigned, unsigned>> uniqueEdges;
    for (size_t i = 0; i < g.indices.size(); i += 3)
        for (size_t j = 0; j < 3; j++) {
            unsigned a = g.indices[i + j], b = g.indices[i + (j + 1) % 3];
            uniqueEdges.emplace(std::min(a, b), std::max(a, b));
        }
    std::vector<unsigned> edgeIndices;
    edgeIndices.reserve(uniqueEdges.size() * 2);
    for (auto [a, b] : uniqueEdges) {
        edgeIndices.push_back(a);
        edgeIndices.push_back(b);
    }
    edgeCount = (GLsizei)edgeIndices.size();
    glGenBuffers(1, &edges);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edges);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, edgeIndices.size() * sizeof(unsigned), edgeIndices.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
    glBindVertexArray(0);
}
Mesh::~Mesh() {
    glDeleteBuffers(1, &edges);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
void Mesh::draw(bool wireframe) const {
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wireframe ? edges : ebo);
    glDrawElements(wireframe ? GL_LINES : GL_TRIANGLES, wireframe ? edgeCount : count,
                   GL_UNSIGNED_INT, nullptr);
}
} // namespace bk
