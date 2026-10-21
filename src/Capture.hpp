#pragma once
#include <cstdio>
#include <fstream>
#include <glad/gl.h>
#include <stdexcept>
#include <string>
#include <vector>
namespace bk {
inline std::vector<unsigned char> pixels(int w, int h) {
    std::vector<unsigned char> p(size_t(w) * h * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, p.data());
    return p;
}
inline void screenshot(const std::string &file, int w, int h) {
    auto p = pixels(w, h);
    std::ofstream f(file, std::ios::binary);
    f << "P6\n" << w << ' ' << h << "\n255\n";
    for (int y = h - 1; y >= 0; y--)
        f.write((char *)p.data() + size_t(y) * w * 3, w * 3);
}
inline std::string shellQuote(const std::string &s) {
#ifdef _WIN32
    if (s.find_first_of("\"%\r\n") != std::string::npos)
        throw std::runtime_error("Unsupported recording path");
    return "\"" + s + "\"";
#else
    std::string r = "'";
    for (char c : s) {
        if (c == '\'')
            r += "'\\''";
        else
            r += c;
    }
    return r + "'";
#endif
}
class Recorder {
    FILE *pipe = nullptr;
    int width, height;
    double next = 0;
    int frames = 0;

  public:
    Recorder(const std::string &exe, const std::string &file, int w, int h, bool hardware = false)
        : width(w), height(h) {
        std::string encoder = hardware
                                  ? " -c:v h264_videotoolbox -b:v 50M -profile:v high -realtime 1"
                                  : " -c:v libx264 -preset ultrafast -crf 16";
        std::string command =
            shellQuote(exe) +
            " -hide_banner -loglevel warning -y -f rawvideo -pixel_format rgb24 -video_size " +
            std::to_string(w) + "x" + std::to_string(h) + " -framerate 30 -i - -vf vflip" +
            encoder +
            " -pix_fmt yuv420p "
            "-movflags +faststart " +
            shellQuote(file);
#ifdef _WIN32
        pipe = _popen(command.c_str(), "wb");
#else
        pipe = popen(command.c_str(), "w");
#endif
        if (!pipe)
            throw std::runtime_error("Cannot start recorder");
    }
    void frame(double wall, int w, int h) {
        if (w != width || h != height)
            throw std::runtime_error("Do not resize the window during recording");
        if (wall < next)
            return;
        auto p = pixels(w, h);
        // Capture wall-clock frames; repeat only if the renderer misses a video tick.
        do {
            if (fwrite(p.data(), 1, p.size(), pipe) != p.size())
                throw std::runtime_error("Video encoder stopped");
            frames++;
            next = frames / 30.0;
        } while (next <= wall);
    }
    ~Recorder() {
        if (pipe) {
#ifdef _WIN32
            _pclose(pipe);
#else
            pclose(pipe);
#endif
        }
    }
};
} // namespace bk
