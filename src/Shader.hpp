#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
namespace bk {
class Shader {
    GLuint id = 0;
    mutable std::unordered_map<std::string, GLint> locations;
    GLint location(const std::string &n) const;

  public:
    Shader(const std::string &vert, const std::string &frag);
    ~Shader();
    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;
    void use() const { glUseProgram(id); }
    void set(const std::string &n, const glm::mat4 &v) const;
    void set(const std::string &n, const glm::vec3 &v) const;
    void set(const std::string &n, float v) const;
    void set(const std::string &n, int v) const;
};
} // namespace bk
