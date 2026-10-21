#include "Mesh.hpp"
#include "KazakhstanBoundary.hpp"
#include <iterator>

namespace bk {
Geometry kazakhstan() {
    Geometry g;
    const unsigned count = std::size(kazakhstanBoundary);
    // Keep the original ground level beneath the launch pad; give the map a rim.
    constexpr float top = -.15f, bottom = -60.f;
    for (float y : {top, bottom})
        for (auto p : kazakhstanBoundary)
            g.vertices.push_back({{p.x, y, p.y}, {0, y == top ? 1.f : -1.f, 0},
                                  {p.x / 10000.f, p.y / 10000.f}});
    for (unsigned i : kazakhstanTriangles)
        g.indices.push_back(i);
    for (size_t i = 0; i < std::size(kazakhstanTriangles); i += 3) {
        g.indices.push_back(count + kazakhstanTriangles[i]);
        g.indices.push_back(count + kazakhstanTriangles[i + 2]);
        g.indices.push_back(count + kazakhstanTriangles[i + 1]);
    }
    for (unsigned i = 0; i < count; ++i) {
        auto a = kazakhstanBoundary[i], b = kazakhstanBoundary[(i + 1) % count];
        glm::vec3 normal = glm::normalize(glm::vec3(a.y - b.y, 0, b.x - a.x));
        unsigned base = static_cast<unsigned>(g.vertices.size());
        g.vertices.push_back({{a.x, top, a.y}, normal, {0, 1}});
        g.vertices.push_back({{a.x, bottom, a.y}, normal, {0, 0}});
        g.vertices.push_back({{b.x, bottom, b.y}, normal, {1, 0}});
        g.vertices.push_back({{b.x, top, b.y}, normal, {1, 1}});
        for (unsigned offset : {0u, 1u, 2u, 0u, 2u, 3u})
            g.indices.push_back(base + offset);
    }
    return g;
}
} // namespace bk
