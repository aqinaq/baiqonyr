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
Geometry cylinder(float top = 1.f, int segments = 96);
Geometry sphere(int rings = 48, int segments = 96);
class Mesh {
    GLuint vao = 0, vbo = 0, ebo = 0, edges = 0;
    GLsizei count = 0, edgeCount = 0;

  public:
    explicit Mesh(const Geometry &data);
    ~Mesh();
    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;
    void draw(bool wireframe = false) const;
};
} // namespace bk
