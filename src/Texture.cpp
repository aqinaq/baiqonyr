#include "Texture.hpp"
#include <random>
#include <vector>
namespace bk {
Texture::Texture() {
    const int size = 1024;
    std::vector<unsigned char> data(size * size * 3);
    std::mt19937 rng(2426);
    for (int y = 0; y < size; y++)
        for (int x = 0; x < size; x++) {
            int noise = int(rng() % 31) - 15;
            bool joint = x < 5 || y < 5;
            for (int c = 0; c < 3; c++)
                data[(y * size + x) * 3 + c] = (unsigned char)(joint ? 80 : 170 + noise);
        }
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
}
Texture::~Texture() {
    glDeleteTextures(1, &id);
}
} // namespace bk
