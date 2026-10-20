#include "Shader.hpp"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <stdexcept>
namespace bk {
static GLuint compile(GLenum type, const std::string &file) {
    std::ifstream f(file);
    if (!f)
        throw std::runtime_error("Cannot read shader: " + file);
    std::stringstream ss;
    ss << f.rdbuf();
    std::string s = ss.str();
    const char *p = s.c_str();
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &p, nullptr);
    glCompileShader(id);
    GLint ok;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[4096];
        glGetShaderInfoLog(id, 4096, nullptr, log);
        glDeleteShader(id);
        throw std::runtime_error(file + ": " + log);
    }
    return id;
}
Shader::Shader(const std::string &v, const std::string &f) {
    GLuint vs = compile(GL_VERTEX_SHADER, v), fs = 0;
    try {
        fs = compile(GL_FRAGMENT_SHADER, f);
    } catch (...) {
        glDeleteShader(vs);
        throw;
    }
    id = glCreateProgram();
    glAttachShader(id, vs);
    glAttachShader(id, fs);
    glLinkProgram(id);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint ok;
    glGetProgramiv(id, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[4096];
        glGetProgramInfoLog(id, 4096, nullptr, log);
        glDeleteProgram(id);
        id = 0;
        throw std::runtime_error(log);
    }
}
Shader::~Shader() {
    if (id)
        glDeleteProgram(id);
}
GLint Shader::location(const std::string &n) const {
    auto it = locations.find(n);
    if (it != locations.end())
        return it->second;
    return locations[n] = glGetUniformLocation(id, n.c_str());
}
void Shader::set(const std::string &n, const glm::mat4 &v) const {
    glUniformMatrix4fv(location(n), 1, GL_FALSE, glm::value_ptr(v));
}
void Shader::set(const std::string &n, const glm::vec3 &v) const {
    glUniform3fv(location(n), 1, glm::value_ptr(v));
}
void Shader::set(const std::string &n, float v) const {
    glUniform1f(location(n), v);
}
void Shader::set(const std::string &n, int v) const {
    glUniform1i(location(n), v);
}
} // namespace bk
