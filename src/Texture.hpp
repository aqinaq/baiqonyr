#pragma once
#include <glad/gl.h>
namespace bk {
class Texture {
    GLuint id = 0;

  public:
    Texture();
    ~Texture();
    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;
    void bind() const {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, id);
    }
};
} // namespace bk
