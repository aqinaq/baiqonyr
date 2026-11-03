#pragma once
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <limits>
namespace bk {
constexpr float pi = 3.14159265358979323846f;
enum class Flight { Ascent, Choosing, Transit, Arrived, Destroyed };
struct Destination {
    const char *name;
    glm::vec3 center, color;
    float radius;
    const char *warning;
    bool dangerous;
};
inline const Destination destinations[] = {
    {"MOON", {-850, 5770, -1750}, {.65,.68,.72}, 55, "Controlled descent and landing on the lunar surface.", false},
    {"MARS", {1200, 5840, -3200}, {.72,.25,.12}, 85, "Controlled descent and landing on Mars.", false},
    {"SUN", {-3600, 6000, -6250}, {1,.55,.12}, 220, "High thermal exposure. Shield endurance uncertain.", true},
    {"VENUS", {2600, 5650, -4500}, {.86,.65,.34}, 105, "Dense atmosphere. Heat and pressure risk.", true},
    {"JUPITER", {-4500, 5900, 1250}, {.74,.57,.39}, 230, "Extreme pressure and turbulent cloud layers.", true},
    {"SATURN", {4750, 5950, 500}, {.82,.72,.48}, 175, "Low-risk observation orbit outside the rings.", false},
    {"MERCURY", {-2100, 5650, -4250}, {.45,.42,.38}, 38, "Surface landing. Keep the sun shield aligned.", false},
    {"URANUS", {2500, 6200, 5000}, {.32,.72,.77}, 105, "Low-risk distant observation orbit.", false},
    {"NEPTUNE", {-2750, 6100, 5750}, {.15,.32,.8}, 110, "Low-risk distant observation orbit.", false}
};
constexpr int destinationCount = sizeof(destinations) / sizeof(destinations[0]);
// Simulation metres and seconds are deliberately compressed for a classroom demo.
struct Mission {
    bool running = false, paused = false, separated = false;
    float clock = -8.f, speed = 1.f, separationTime = 24.f;
    Flight flight = Flight::Ascent;
    int destination = -1;
    float travel = 0, aftermath = 0, resultDelay = 0;
    std::vector<glm::vec3> route;
    float routeLength = 0;
    static constexpr float orbitTime = 100.f, travelDuration = 18.f;
    bool inSpace() const { return flight != Flight::Ascent; }
    bool selectDestination(int index) {
        if (flight != Flight::Choosing || index < 0 || index >= destinationCount) return false;
        if (!planRoute(index)) return false;
        destination = index;
        travel = 0;
        flight = Flight::Transit;
        return true;
    }
    float progress() const { return std::clamp(travel / travelDuration, 0.f, 1.f); }
    float danger() const {
        return destination >= 0 && destinations[destination].dangerous
            ? std::clamp((progress() - .45f) / .55f, 0.f, 1.f) : 0.f;
    }
    glm::vec3 position() const {
        if (route.empty()) return {0, altitudeAt(std::min(clock, orbitTime)), 0};
        if (canLand() && progress() > .75f) {
            float t = (progress() - .75f) / .25f;
            t = t*t*(3.f-2.f*t);
            return glm::mix(route.back(), landingPoint(), t);
        }
        float distance = std::min(progress() / (canLand() ? .75f : 1.f), 1.f) * routeLength;
        for (std::size_t i=1; i<route.size(); ++i) {
            float length = glm::length(route[i]-route[i-1]);
            if (distance <= length) return glm::mix(route[i-1],route[i],distance/std::max(length,.001f));
            distance -= length;
        }
        return route.back();
    }
    static float clearance(int i) { return i == 5 ? 330.f : destinations[i].radius + 45.f; }
    static bool clearSegment(glm::vec3 a, glm::vec3 b) {
        glm::vec3 delta=b-a;
        float length2=glm::dot(delta,delta);
        for (int i=0; i<destinationCount; ++i) {
            float t=length2>0 ? std::clamp(glm::dot(destinations[i].center-a,delta)/length2,0.f,1.f) : 0;
            if (glm::length(a+t*delta-destinations[i].center) < clearance(i)) return false;
        }
        return true;
    }
    bool planRoute(int index) {
        glm::vec3 start(0,altitudeAt(orbitTime),0);
        const auto &d=destinations[index];
        glm::vec3 end=d.center+glm::normalize(start-d.center)*(clearance(index)+35.f);
        std::vector<glm::vec3> nodes{start,end};
        // Visibility graph: connect only segments outside every planet and ring envelope.
        for (int i=0; i<destinationCount; ++i)
            for (int axis=0; axis<3; ++axis)
                for (float sign : {-1.f,1.f}) {
                    auto point=destinations[i].center;
                    point[axis]+=sign*(clearance(i)*1.8f+100.f);
                    nodes.push_back(point);
                }
        std::vector<float> cost(nodes.size(),std::numeric_limits<float>::infinity());
        std::vector<int> previous(nodes.size(),-1);
        std::vector<bool> visited(nodes.size(),false);
        cost[0]=0;
        for (std::size_t step=0; step<nodes.size(); ++step) {
            int u=-1;
            for (std::size_t i=0; i<nodes.size(); ++i)
                if (!visited[i] && (u<0 || cost[i]<cost[u])) u=int(i);
            if (u<0 || !std::isfinite(cost[u])) break;
            if (u==1) break;
            visited[u]=true;
            for (std::size_t v=0; v<nodes.size(); ++v) {
                float next=cost[u]+glm::length(nodes[v]-nodes[u]);
                if (!visited[v] && next<cost[v] && clearSegment(nodes[u],nodes[v])) {
                    cost[v]=next; previous[v]=u;
                }
            }
        }
        if (!std::isfinite(cost[1])) return false;
        route.clear();
        for (int at=1; at>=0; at=previous[at]) route.push_back(nodes[at]);
        std::reverse(route.begin(),route.end()); routeLength=cost[1];
        return true;
    }
    bool canLand() const { return destination == 0 || destination == 1 || destination == 6; }
    glm::vec3 landingNormal() const {
        return glm::normalize(route.back() - destinations[destination].center);
    }
    glm::vec3 landingPoint() const {
        return destinations[destination].center + landingNormal() * (destinations[destination].radius + .15f);
    }
    bool resultReady() const { return resultDelay >= 5.f; }
    void launch() {
        if (!running) {
            running = true;
            clock = -8.f;
        }
    }
    void reset() { *this = Mission{}; }
    void update(float dt) {
        if (!running || paused) return;
        float step = std::max(0.f, dt) * speed;
        if (flight == Flight::Destroyed || flight == Flight::Arrived) {
            resultDelay += std::max(0.f, dt); // Real seconds, independent of playback speed.
            aftermath = std::min(aftermath + std::max(0.f, dt), 12.f);
            return;
        }
        if (flight == Flight::Choosing || flight == Flight::Arrived) return;
        if (flight == Flight::Transit) {
            travel = std::min(travel + step, travelDuration);
            if (travel >= travelDuration)
                flight = destinations[destination].dangerous ? Flight::Destroyed : Flight::Arrived;
            return;
        }
        clock = std::min(clock + step, orbitTime);
        if (clock >= 24.f && !separated) {
            separated = true;
            separationTime = 24.f;
        }
        if (clock >= orbitTime) flight = Flight::Choosing;
    }

    void separate() {
        if (clock >= 5.f && !separated) {
            separated = true;
            separationTime = clock;
        }
    }
    float altitudeAt(float t) const {
        t = std::max(0.f, t);
        return .55f * t * t;
    }
    float altitude() const { return position().y; }
    float velocity() const { return inSpace() ? 0.f : 1.1f * std::max(0.f, clock); }
    bool ignition() const { return running && clock >= -2.f && (flight == Flight::Ascent || flight == Flight::Transit); }
    glm::mat4 rocket() const { auto transform = glm::translate(glm::mat4(1), position());
        if (destination >= 0) {
            glm::vec3 direction = glm::normalize(destinations[destination].center - glm::vec3(0, altitudeAt(orbitTime), 0));
            if (canLand()) {
                float turn = std::clamp((progress()-.55f)/.20f, 0.f, 1.f);
                turn = turn*turn*(3.f-2.f*turn);
                glm::vec3 outward = landingNormal();
                float cosine = std::clamp(glm::dot(direction,outward),-1.f,1.f);
                glm::vec3 tangent = outward-direction*cosine;
                if (glm::length(tangent)<.0001f)
                    tangent = glm::cross(direction, std::abs(direction.y)<.9f ? glm::vec3(0,1,0) : glm::vec3(1,0,0));
                tangent=glm::normalize(tangent);
                float angle=std::acos(cosine)*turn;
                direction=glm::normalize(direction*std::cos(angle)+tangent*std::sin(angle));
            }
            glm::vec3 cross = glm::cross(glm::vec3(0,1,0), direction);
            glm::vec3 axis = glm::length(cross) > .0001f ? glm::normalize(cross) : glm::vec3(1,0,0);
            transform = glm::rotate(transform, std::acos(std::clamp(direction.y, -1.f, 1.f)) * std::min(progress()*5.f, 1.f), axis);
        }
        return transform; }
    glm::mat4 booster(int i) const {
        float a = float(i) * pi * .5f;
        glm::vec3 radial(std::cos(a), 0, std::sin(a));
        auto local = glm::translate(glm::mat4(1), radial * 1.72f + glm::vec3(0, 3.8f, 0));
        if (!separated)
            return rocket() * local;
        float t = std::max(0.f, clock - separationTime);
        // Detach in world space, preserve position and initial vertical velocity.
        glm::vec3 p =
            radial * (1.72f + 1.8f * t) +
            glm::vec3(
                0, 3.8f + altitudeAt(separationTime) + 1.1f * separationTime * t - 1.8f * t * t, 0);
        p.y = std::max(.8f, p.y);
        return glm::rotate(glm::translate(glm::mat4(1), p), t * .35f,
                           glm::vec3(radial.z, 0, -radial.x));
    }
    std::string phase() const {
        if (flight == Flight::Choosing) return "AWAITING DESTINATION";
        if (flight == Flight::Transit && canLand() && progress() > .75f) return "LANDING / CONTROLLED DESCENT";
        if (flight == Flight::Transit) return danger() > .4f ? "WARNING / HULL FAILURE" : "INTERPLANETARY TRANSFER";
        if (flight == Flight::Arrived) return resultReady() ? (canLand() ? "LANDED" : "ORBIT ACHIEVED") : "STABILIZING";
        if (flight == Flight::Destroyed) return resultReady() ? "SIGNAL LOST" : "RECONNECTING";
        if (!running)
            return "READY FOR LAUNCH";
        if (clock < -2)
            return "COUNTDOWN";
        if (clock < 0)
            return "ENGINE IGNITION";
        if (separated)
            return "CORE STAGE ASCENT";
        return "POWERED ASCENT";
    }
};
inline float atmosphere(float altitude) {
    return std::clamp(altitude / 450.f, 0.f, 1.f);
}
} // namespace bk
