#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>
namespace bk {
struct Vertex {
    glm::vec3 position, normal;
    glm::vec2 uv;
};
struct Geometry {
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
};
Geometry cube();
Geometry cylinder(float top = 1.f, int segments = 48);
Geometry sphere(int rings = 18, int segments = 36);
class Mesh {
    GLuint vao = 0, vbo = 0, ebo = 0;
    GLsizei count = 0;

  public:
    explicit Mesh(const Geometry &data);
    ~Mesh();
    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;
    void draw() const;
};
} // namespace bk
