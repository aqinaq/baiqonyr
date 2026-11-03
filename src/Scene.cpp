#include "Scene.hpp"
namespace bk {
static glm::mat4 trs(glm::vec3 p, glm::vec3 s) {
    return glm::scale(glm::translate(glm::mat4(1), p), s);
}
static Object obj(int mesh, glm::vec3 p, glm::vec3 s, glm::vec3 c, int mat = 0, float shine = 32,
                  float emission = 0) {
    return {mesh, trs(p, s), c, mat, shine, emission};
}
Scene::Scene(const std::string &path)
    : shader(path + "/scene.vert", path + "/scene.frag"),
      sky(path + "/sky.vert", path + "/sky.frag") {
    glGenVertexArrays(1, &skyVAO);
    glm::vec3 sand(.33, .26, .14), concreteColor(.48, .49, .46), steel(.095, .16, .18),
        orange(.8, .21, .055), white(.82, .85, .79), dark(.035, .055, .065);
    ground.push_back(obj(4, {0, 0, 0}, {1, 1, 1}, sand, 2));
    ground.push_back(obj(0, {0, -.05f, 0}, {42, .3f, 42}, concreteColor, 1));
    ground.push_back(obj(0, {0, .22f, 0}, {9, .4f, 9}, dark));
    ground.push_back(obj(0, {0, .1f, 90}, {12, .13f, 150}, {.12, .13, .13}, 1));
    for (int z = 20; z < 160; z += 9)
        ground.push_back(obj(0, {0, .18f, float(z)}, {.15, .02, 4}, {.8, .72, .4}));
    // Flame trench, launch supports and hazard bars.
    ground.push_back(obj(0, {0, .2f, -11}, {4, .4, 17}, {.06, .065, .055}));
    for (int i = 0; i < 4; i++) {
        float a = i * pi * .5f;
        ground.push_back(
            obj(0, {std::cos(a) * 3, 1.25f, std::sin(a) * 3}, {1.1f, 2.5f, 1.1f}, steel));
    }
    for (int i = 0; i < 12; i++)
        ground.push_back(obj(0, {-6 + float(i), .13f, 7}, {.5, .02, .7}, i % 2 ? orange : dark));
    // Four columns and cross bracing form the service gantry.
    for (float x : {-9.f, -6.f})
        for (float z : {-2.f, 2.f})
            ground.push_back(obj(0, {x, 10, z}, {.38, 20, .38}, steel));
    for (int level = 0; level < 6; level++) {
        float y = 2 + level * 3.2f;
        ground.push_back(obj(0, {-7.5, y, 0}, {4, .22, 5}, steel));
        for (float z : {-2.f, 2.f}) {
            auto brace = obj(0, {-7.5f, y + 1.5f, z}, {.19f, 4.2f, .19f}, orange);
            brace.transform = glm::translate(glm::mat4(1), {-7.5f, y + 1.5f, z}) *
                              glm::rotate(glm::mat4(1), -.75f, {0, 0, 1}) *
                              glm::scale(glm::mat4(1), {.19f, 4.2f, .19f});
            ground.push_back(brace);
            ground.push_back(obj(0, {-7.5f, y + 1.1f, z}, {3.7f, .1f, .1f}, orange));
        }
    }
    ground.push_back(obj(0, {-3.8, 15.4, 0}, {5, .35, 1.1}, steel));
    ground.push_back(obj(0, {-7.5, 20, 0}, {5, .5, 6}, orange));
    ground.push_back(obj(3, {-8, 17, 5}, {.35, .35, .35}, {.55, .8, 1}, 0, 32, 3));
    // Control buildings, tanks, radome and perimeter lamps.
    for (int i = 0; i < 3; i++) {
        float x = 20 + i * 9.f;
        ground.push_back(obj(0, {x, 2, -22}, {7, 4, 10}, white, 1));
        ground.push_back(obj(0, {x, 4.1f, -22}, {7.5, .3, 10.5}, steel));
        for (int j = 0; j < 4; j++)
            ground.push_back(obj(0, {x - 2.4f + j * 1.6f, 2.5f, -16.98f}, {1, .9, .06},
                                 {.12, .35, .43}, 0, 64, .3f));
    }
    for (int i = 0; i < 3; i++) {
        ground.push_back(obj(1, {18 + float(i) * 4, 3, 12}, {1.5, 6, 1.5}, white));
        ground.push_back(obj(3, {18 + float(i) * 4, 6.01f, 12}, {1.49, .65, 1.49}, white));
        ground.push_back(obj(1, {18 + float(i) * 4, 2.7f, 12}, {1.52, .35, 1.52}, orange));
    }
    ground.push_back(obj(1, {-28, 2, -26}, {5, 4, 5}, concreteColor, 1));
    ground.push_back(obj(3, {-28, 4, -26}, {5.2, 5.2, 5.2}, white));
    for (int i = 0; i < 18; i++) {
        float a = i * 2 * pi / 18;
        glm::vec3 p(37 * std::cos(a), 2, 37 * std::sin(a));
        ground.push_back(obj(1, p, {.08, 4, .08}, steel));
        ground.push_back(obj(3, p + glm::vec3(0, 2, 0), {.18, .18, .18}, orange, 0, 32, 1.5));
    }
    buildEnvironment();
    // All rocket parts inherit the animated core transform.
    rocketParts.push_back(obj(1, {0, 8.2, 0}, {1.16, 12, 1.16}, white, 0, 90));
    rocketParts.push_back(obj(1, {0, 4, 0}, {1.18, 2.4, 1.18}, orange));
    rocketParts.push_back(obj(1, {0, 13.8, 0}, {1.18, .25, 1.18}, dark));
    rocketParts.push_back(obj(1, {0, 14.8, 0}, {1.25, 2, 1.25}, white, 0, 90));
    rocketParts.push_back(obj(2, {0, 17, 0}, {1.25, 2.4, 1.25}, white, 0, 90));
    rocketParts.push_back(obj(1, {0, 18.8, 0}, {.055, 1.3, .055}, steel));
    rocketParts.push_back(obj(1, {0, 1.65, 0}, {.62, 1, .62}, dark));
    for (float y : {6.f, 10.5f, 12.4f})
        rocketParts.push_back(obj(1, {0, y, 0}, {1.175, .1, 1.175}, steel));
    // Four landing struts and broad feet meet the surface below the engine bell.
    for (int i=0; i<4; ++i) {
        float angle = i*pi*.5f;
        glm::vec3 radial(std::cos(angle),0,std::sin(angle));
        rocketParts.push_back(obj(0, radial*1.5f+glm::vec3(0,1.1f,0), {.18f,2.0f,.18f}, steel));
        rocketParts.push_back(obj(0, radial*1.5f+glm::vec3(0,.1f,0), {.7f,.2f,.7f}, dark));
    }
    // Narrow turquoise panels on four sides refer to the Kazakhstan palette.
    for (int i = 0; i < 4; i++) {
        float a = i * pi * .5f;
        auto o = obj(0, {1.16f * std::cos(a), 11.5f, 1.16f * std::sin(a)}, {.035f, 1.6f, .48f},
                     {.04, .55, .65});
        o.transform =
            glm::translate(glm::mat4(1), {1.16f * std::cos(a), 11.5f, 1.16f * std::sin(a)}) *
            glm::rotate(glm::mat4(1), -a, {0, 1, 0}) *
            glm::scale(glm::mat4(1), {.035f, 1.6f, .48f});
        rocketParts.push_back(o);
    }
    boosterParts.push_back(obj(1, {0, 0, 0}, {.64, 5.4, .64}, white, 0, 64));
    boosterParts.push_back(obj(2, {0, 3.35, 0}, {.64, 1.3, .64}, white, 0, 64));
    boosterParts.push_back(obj(1, {0, -1.8, 0}, {.66, 1.6, .66}, orange));
    boosterParts.push_back(obj(1, {0, -3, 0}, {.38, .7, .38}, dark));
    boosterParts.push_back(obj(1, {0, 1.8, 0}, {.65, .13, .65}, steel));
}
Scene::~Scene() {
    glDeleteVertexArrays(1, &skyVAO);
}
void Scene::draw(const Object &o, const glm::mat4 &parent) {
    shader.set("model", parent * o.transform);
    shader.set("baseColor", o.color);
    shader.set("material", o.material);
    shader.set("shininess", o.shine);
    shader.set("emission", o.emission);
    const Mesh *meshes[] = {&box, &tube, &cone, &ball, &terrain, &rock, &scrub, &pipe, &ridges, &rings};
    meshes[o.mesh]->draw(wireframe);
}
void Scene::render(const Mission &m, Camera &camera, int width, int height, float realTime) {
    glm::mat4 view = camera.view(m, realTime),
              projection =
                  glm::perspective(glm::radians(48.f), float(width) / height, camera.distance > 1000 ? 10.f : .1f, 30000.f);
    camera.viewProjection = projection * view;
    float a = atmosphere(m.altitude());
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    sky.use();
    sky.set("invVP", glm::inverse(projection * view));
    sky.set("eye", camera.eye);
    sky.set("altitude", a);
    sky.set("night", night ? 1.f : 0.f);
    sky.set("time", realTime);
    glBindVertexArray(skyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    shader.use();
    concrete.bind();
    shader.set("concrete", 0);
    shader.set("time", realTime);
    shader.set("view", view);
    shader.set("projection", projection);
    shader.set("eye", camera.eye);
    shader.set("sunDir", glm::vec3(-.6, .7, -.8));
    shader.set("night", night ? 1.f : 0.f);
    shader.set("altitude", a);
    shader.set("flagReveal", glm::smoothstep(800.f, 4500.f, m.altitude()));
    shader.set("lightsOn", lights ? 1 : 0);
    shader.set("enginePos", glm::vec3(m.rocket() * glm::vec4(0, 1.4f, 0, 1)));
    shader.set("enginePower", m.ignition() ? 9.f : 0.f);
    for (const auto &o : ground)
        if (!m.inSpace() && (o.mesh == 4 || m.altitude() < 1000.f)) draw(o, glm::mat4(1));
    if (m.altitude() > 350.f) {
// The Kazakhstan terrain remains our home world at every altitude.
        for (int i = 0; i < destinationCount; ++i) {
            const auto &d = destinations[i];
            draw(obj(3, d.center, glm::vec3(m.flight == Flight::Choosing ? std::max(d.radius, 260.f) : d.radius), d.color, i == 2 ? 5 : 10+i,
                     8, i == 2 ? 2.5f : .12f), glm::mat4(1));
            if (i == 5) {
                Object ring=obj(9,d.center,{210,210,210},{.72,.62,.43},19,4,.2f);
                ring.transform=glm::translate(glm::mat4(1),d.center)*
                    glm::rotate(glm::mat4(1),.24f,glm::vec3(1,0,0))*glm::scale(glm::mat4(1),glm::vec3(m.flight == Flight::Choosing ? 360.f : 210.f));
                draw(ring,glm::mat4(1));
            }
        }
    }
    if (m.flight != Flight::Destroyed) {
        for (auto o : rocketParts) {
            o.color = glm::mix(o.color, glm::vec3(1,.16f,.025f), m.danger()*.65f);
            o.emission += m.danger()*1.5f;
            draw(o, m.flight == Flight::Choosing
                        ? glm::scale(m.rocket(), glm::vec3(65.f)) : m.rocket());
        }
    } else {
        for (size_t i = 0; i < rocketParts.size(); ++i) {
            float angle = float(i)*2.39996f;
            glm::vec3 drift(std::cos(angle), std::sin(angle*.7f), std::sin(angle));
            glm::mat4 debris = glm::translate(glm::mat4(1), drift*m.aftermath*8.f);
            debris = debris * m.rocket() * glm::rotate(glm::mat4(1), m.aftermath*(.3f+i*.07f), glm::normalize(drift));
            draw(rocketParts[i], debris);
        }
    }
    for (int i = 0; i < 4; i++)
        for (const auto &o : boosterParts)
            draw(o, m.booster(i));
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
}
} // namespace bk
