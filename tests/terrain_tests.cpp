#include "Mesh.hpp"
#include "KazakhstanBoundary.hpp"
#include <cmath>
#include <iostream>
#include <iterator>
#include <stdexcept>

static void require(bool ok, const char *message) {
    if (!ok)
        throw std::runtime_error(message);
}

static bool inside(glm::vec2 p) {
    bool result = false;
    const size_t n = std::size(bk::kazakhstanBoundary);
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        auto a = bk::kazakhstanBoundary[i], b = bk::kazakhstanBoundary[j];
        if ((a.y > p.y) != (b.y > p.y) &&
            p.x < (b.x - a.x) * (p.y - a.y) / (b.y - a.y) + a.x)
            result = !result;
    }
    return result;
}

int main() {
    try {
        auto g = bk::kazakhstan();
        const size_t n = std::size(bk::kazakhstanBoundary);
        require(g.indices.size() == 6 * (n - 2) + 6 * n, "Closed terrain face count");
        double polygonArea = 0, triangleArea = 0;
        for (size_t i = 0; i < n; ++i) {
            auto a = bk::kazakhstanBoundary[i], b = bk::kazakhstanBoundary[(i + 1) % n];
            polygonArea += double(a.y) * b.x - double(a.x) * b.y;
        }
        for (size_t i = 0; i < g.indices.size(); i += 3) {
            for (size_t j = 0; j < 3; ++j)
                require(g.indices[i + j] < g.vertices.size(), "Valid vertex index");
            auto a = g.vertices[g.indices[i]], b = g.vertices[g.indices[i + 1]],
                 c = g.vertices[g.indices[i + 2]];
            auto cross = glm::cross(b.position - a.position, c.position - a.position);
            require(glm::dot(cross, a.normal) > 0, "Outward winding and nondegenerate faces");
            if (i < std::size(bk::kazakhstanTriangles)) {
                triangleArea += cross.y;
                auto center = (a.position + b.position + c.position) / 3.f;
                require(inside({center.x, center.z}), "Top triangles stay inside Kazakhstan");
            }
        }
        require(std::abs(triangleArea - polygonArea) < polygonArea * 1e-5,
                "Triangulation covers the boundary area");
        // Pad, access road and surrounding ridges must all remain on the map.
        for (auto p : {glm::vec2(0, 0), glm::vec2(0, 165), glm::vec2(-550, -550),
                       glm::vec2(550, 550), glm::vec2(-550, 550), glm::vec2(550, -550)})
            require(inside(p), "Launch complex remains on land");
        std::cout << "PASS Kazakhstan terrain topology, winding, area and launch location\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
