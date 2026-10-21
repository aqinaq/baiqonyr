#include "Scene.hpp"
#include <random>

namespace bk {
Geometry steppeRidges() {
    Geometry g;
    constexpr int segments = 96, bands = 6;
    glm::vec3 points[bands][segments];
    for (int band = 0; band < bands; ++band)
        for (int i = 0; i < segments; ++i) {
            float a = 2 * pi * i / segments;
            float ripple = std::sin(a * 7 + band) * 18 + std::sin(a * 13 - band) * 9;
            float radius = 240 + band * 110.f + ripple;
            float height = band == 0 || band == bands - 1 ? -.2f :
                10 + band * 7.f + 17 * std::sin(a * 5 + band * 1.8f) +
                12 * std::sin(a * 11 + band) + 8 * std::cos(a * 19);
            points[band][i] = {radius * std::cos(a), std::max(-.2f, height * .65f),
                               radius * std::sin(a)};
        }
    auto triangle = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c) {
        glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
        if (normal.y < 0) {
            std::swap(b, c);
            normal = -normal;
        }
        for (auto p : {a, b, c}) {
            g.indices.push_back(static_cast<unsigned>(g.vertices.size()));
            g.vertices.push_back({p, normal, {p.x / 100, p.z / 100}});
        }
    };
    for (int band = 0; band < bands - 1; ++band)
        for (int i = 0; i < segments; ++i) {
            int next = (i + 1) % segments;
            triangle(points[band][i], points[band + 1][i], points[band][next]);
            triangle(points[band][next], points[band + 1][i], points[band + 1][next]);
        }
    return g;
}

void Scene::buildEnvironment() {
    const glm::vec3 steel(.12, .19, .21), pale(.63, .65, .57), rust(.43, .18, .075),
        asphalt(.095, .105, .10), glass(.065, .24, .29), yellow(.83, .55, .13);
    auto add = [&](int mesh, glm::vec3 p, glm::vec3 scale, glm::vec3 color,
                   int material = 0, float emission = 0.f) -> Object & {
        ground.push_back({mesh, glm::scale(glm::translate(glm::mat4(1), p), scale),
                          color, material, 12, emission});
        return ground.back();
    };
    auto beam = [&](glm::vec3 a, glm::vec3 b, float width, glm::vec3 color) {
        glm::vec3 y = glm::normalize(b - a);
        glm::vec3 x = glm::normalize(glm::cross(
            std::abs(y.y) > .95f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0), y));
        glm::vec3 z = glm::cross(x, y);
        glm::mat4 model(1);
        model[0] = glm::vec4(x * width, 0);
        model[1] = glm::vec4(b - a, 0);
        model[2] = glm::vec4(z * width, 0);
        model[3] = glm::vec4((a + b) * .5f, 1);
        ground.push_back({0, model, color, 0, 16, 0});
    };

    add(8, {0, 0, 0}, {1, 1, 1}, {.29, .26, .19}, 2);
    // A service road and railway lead the eye from the pad into the compound.
    add(0, {0, -.025, -53}, {172, .12, 8}, asphalt);
    for (int x = -80; x <= 80; x += 10)
        add(0, {float(x), .045, -53}, {4, .02, .16}, yellow);
    add(0, {-48, -.025, 35}, {7, .12, 180}, asphalt);
    for (float x : {-52.f, -44.f})
        add(0, {x, -.02, 35}, {.4, .10, 180}, {.45, .41, .3});
    for (float x : {-23.f, -20.f})
        add(0, {x, .07, -110}, {.13, .19, 155}, steel);
    for (int i = 0; i < 42; ++i)
        add(0, {-21.5, -.01, -34 - i * 3.6f}, {4.5, .10, .34}, {.22, .16, .10});

    // Assembly hangar: ribbed walls, large doors, clerestory windows and roof vents.
    add(0, {-51, 5.9, -83}, {35, 12, 23}, pale);
    add(0, {-51, 12.15, -83}, {37, .65, 25}, steel);
    add(0, {-51, 3.8, -71.42}, {21, 7.6, .20}, steel);
    for (int i = 0; i < 9; ++i) {
        float x = -67 + i * 4.f;
        add(0, {x, 6, -71.25}, {.16, 12, .16}, {.36, .41, .39});
        add(0, {x, 10, -71.22}, {2.8, 1.15, .12}, glass, 0, .18);
    }
    for (float x : {-61.f, -51.f, -41.f}) {
        add(0, {x, 12.8, -83}, {3, .8, 5}, {.36, .41, .39});
        add(7, {x, 13.7, -83}, {.6, 1.2, .6}, pale);
    }
    // Fuel farm on a separate concrete apron.
    add(0, {55, -.04, -72}, {32, .2, 27}, {.41, .41, .35}, 1);
    for (float x : {46.f, 56.f, 66.f}) {
        add(7, {x, 5.4, -77}, {3.4, 11, 3.4}, pale);
        add(5, {x, 10.9, -77}, {3.4, 1.1, 3.4}, pale);
        add(7, {x, 7.8, -77}, {3.43, .7, 3.43}, rust);
        beam({x, 1.1, -73}, {x, 1.1, -64}, .35f, steel);
    }
    beam({44, 1.1, -64}, {68, 1.1, -64}, .45f, steel);
    for (float x : {45.f, 55.f, 65.f})
        add(0, {x, .45, -64}, {.8, .9, 1.2}, pale);

    // Open lattice communications mast, alternating safety bands and red beacon.
    for (int level = 0; level < 6; ++level) {
        float y = level * 5.f;
        float r = 2.8f - level * .32f, next = r - .32f;
        auto color = level % 2 ? pale : rust;
        for (int side = 0; side < 4; ++side) {
            float a = pi * .5f * side, b = a + pi * .5f;
            glm::vec3 lo(-77 + r * std::cos(a), y, -42 + r * std::sin(a));
            glm::vec3 hi(-77 + next * std::cos(a), y + 5, -42 + next * std::sin(a));
            glm::vec3 across(-77 + next * std::cos(b), y + 5, -42 + next * std::sin(b));
            beam(lo, hi, .18f, color);
            beam(lo, across, .11f, color);
            beam(hi, across, .12f, color);
        }
    }
    beam({-77, 28, -42}, {-77, 35, -42}, .16f, steel);
    add(5, {-77, 35, -42}, {.32, .32, .32}, {1, .08, .025}, 0, 3);
    add(5, {-75.8, 24, -42}, {.3, 1.7, 1.7}, pale);

    // Fence follows the rear boundary, with a gap for the service road.
    for (int i = 0; i <= 28; ++i) {
        float x = -84 + i * 6.f;
        add(7, {x, 1.4, -35}, {.065, 3.1, .065}, steel);
        if (i < 28 && (x < -54 || x >= -42))
            for (float y : {.65f, 1.55f, 2.65f})
                beam({x, y, -35}, {x + 6, y, -35}, .035f, steel);
    }
    // Utility poles with sagging cables along the western road.
    for (int i = 0; i < 6; ++i) {
        float z = -110 + i * 28.f;
        add(7, {-57, 5.3, z}, {.16, 11, .16}, {.28, .23, .16});
        add(0, {-57, 10.3, z}, {3.4, .18, .2}, steel);
        for (float x : {-58.3f, -55.7f}) {
            add(7, {x, 10.6, z}, {.14, .42, .14}, pale);
            if (i < 5)
                for (int segment = 0; segment < 4; ++segment) {
                    float t = segment / 4.f, u = (segment + 1) / 4.f;
                    beam({x, 10.8f - 3.2f * t * (1 - t), z + 28 * t},
                         {x, 10.8f - 3.2f * u * (1 - u), z + 28 * u}, .035f, steel);
                }
        }
    }
    // Containers and parked service trucks add human scale near the buildings.
    for (int i = 0; i < 3; ++i) {
        float x = -30 - i * 7.f;
        auto color = i % 2 ? rust : glm::vec3(.09, .29, .32);
        add(0, {x, 1.4, -62}, {5.8, 2.9, 3}, color);
        for (int rib = 0; rib < 7; ++rib)
            add(0, {x - 2.5f + rib * .8f, 1.4, -60.46}, {.09, 2.7, .08}, color * .7f);
    }
    for (auto p : {glm::vec3(-39, 0, -43), glm::vec3(31, 0, -43)}) {
        add(0, p + glm::vec3(0, 1, 0), {2.5, .6, 6}, steel);
        add(0, p + glm::vec3(0, 2, 1.7), {2.4, 2, 2}, yellow);
        add(0, p + glm::vec3(0, 2.3, 2.72), {2.05, .85, .04}, glass);
        add(0, p + glm::vec3(0, 1.9, -1.4), {2.4, 1.4, 3.7}, pale);
        for (float x : {-1.25f, 1.25f})
            for (float z : {-2.f, 2.f})
                add(5, p + glm::vec3(x, .65, z), {.32, .65, .65}, {.025, .03, .028});
    }
    // Deterministic, sparse steppe scrub and stones, kept clear of paved areas.
    std::mt19937 rng(2426);
    auto random = [&]() { return float(rng() % 10000) / 10000.f; };
    for (int i = 0; i < 42; ++i) {
        float x = -85 + random() * 170, z = -33 + random() * 3;
        if (std::abs(x + 48) < 6 || std::abs(x + 21.5f) < 4)
            continue;
        float size = .35f + random() * .55f;
        add(6, {x, size * .3f, z}, {size, size * .9f, size * .7f}, {.26, .28, .12});
    }
    for (int i = 0; i < 240; ++i) {
        float x = (random() - .5f) * 420, z = (random() - .5f) * 420;
        if (std::abs(x) < 88 && z > -102 && z < 45)
            continue;
        if ((std::abs(x) < 9 && z > 0 && z < 170) || std::abs(x + 48) < 6 ||
            (std::abs(x + 21.5f) < 4 && z < -30))
            continue;
        float size = .3f + random() * .85f;
        if (i % 4 == 0)
            add(5, {x, size * .16f, z}, {size * 1.5f, size * .5f, size},
                {.30, .28, .22}, 2);
        else {
            auto color = glm::mix(glm::vec3(.18, .22, .095), glm::vec3(.37, .32, .15), random());
            add(6, {x, size * .35f, z}, {size, size, size * .8f}, color);
            add(6, {x + size * .6f, size * .2f, z + .2f}, {size * .6f, size * .7f, size * .5f}, color);
        }
    }
}
} // namespace bk
