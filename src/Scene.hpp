#pragma once
#include "Camera.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include <string>
namespace bk {
struct Object {
    int mesh;
    glm::mat4 transform;
    glm::vec3 color;
    int material = 0;
    float shine = 32, emission = 0;
};
class Scene {
    Mesh box{cube()}, tube{cylinder()}, cone{cylinder(0)}, ball{sphere()};
    Shader shader, sky;
    Texture concrete;
    GLuint skyVAO = 0;
    std::vector<Object> ground, rocketParts, boosterParts;
    void draw(const Object &o, const glm::mat4 &parent);

  public:
    bool lights = true, night = false, wireframe = false;
    explicit Scene(const std::string &path);
    ~Scene();
    void render(const Mission &mission, Camera &camera, int width, int height, float realTime);
    size_t objects() const { return ground.size() + rocketParts.size() + 4 * boosterParts.size(); }
};
} // namespace bk
