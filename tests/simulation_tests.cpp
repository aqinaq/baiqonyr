#include "Simulation.hpp"
#include <iostream>
#include <stdexcept>
using namespace bk;
void require(bool value, const char *name) {
    if (!value)
        throw std::runtime_error(name);
    std::cout << "PASS " << name << '\n';
}
float distance(const glm::mat4 &a, const glm::mat4 &b) {
    float d = 0;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            d += std::abs(a[c][r] - b[c][r]);
    return d;
}
int main() {
    try {
        Mission m;
        require(!m.running && m.altitude() == 0, "idle is grounded");
        m.launch();
        m.update(6);
        require(m.clock == -2 && m.ignition(), "ignition begins at T minus 2");
        m.paused = true;
        m.update(30);
        require(m.clock == -2, "pause preserves mission time");
        m.paused = false;
        m.update(12);
        require(m.altitude() > 0, "ascent after countdown");
        auto before = m.booster(0);
        m.separate();
        require(distance(before, m.booster(0)) < .001f, "separation preserves world transform");
        m.update(10);
        require(m.booster(0)[3].x > before[3].x, "detached boosters move outward");
        Mission a, b;
        a.launch();
        b.launch();
        for (int i = 0; i < 600; i++)
            a.update(1.f / 60);
        for (int i = 0; i < 300; i++)
            b.update(1.f / 30);
        require(std::abs(a.altitude() - b.altitude()) < .01f,
                "time integration is frame-rate independent");
        m.reset();
        require(!m.separated && !m.running && m.clock == -8, "reset clears mission state");
        m.separate();
        require(!m.separated, "premature separation rejected");
        m.launch();
        m.update(33);
        require(m.separated, "automatic separation occurs");
        require(atmosphere(900) == 1 && atmosphere(-1) == 0, "atmosphere factor bounded");
        for (int i = 0; i < 4; i++)
            require(std::isfinite(m.booster(i)[3].y), "booster transform finite");
        Mission trip;
        require(!trip.selectDestination(0), "cannot choose destination before orbit");
        trip.launch();
        trip.update(1000);
        require(trip.flight == Flight::Choosing && trip.clock == Mission::orbitTime, "ascent stops at navigation gate");
        auto holding = trip.position();
        trip.update(1000);
        require(glm::length(trip.position()-holding) == 0, "waiting does not drift indefinitely");
        require(!trip.selectDestination(-1) && !trip.selectDestination(destinationCount), "invalid destinations rejected");
        for (int i = 0; i < destinationCount; ++i) {
            trip.reset(); trip.launch(); trip.update(Mission::orbitTime+8);
            require(trip.selectDestination(i), "destination accepted at gate");
            for (std::size_t j=1; j<trip.route.size(); ++j)
                require(Mission::clearSegment(trip.route[j-1],trip.route[j]),
                        "entire route clears every planet and ring envelope");
            trip.paused = true; trip.update(50);
            require(trip.progress() == 0, "pause freezes transfer");
            trip.paused = false;
            trip.update(9);
            require(trip.progress() == .5f && glm::length(trip.position()-holding) > 1, "ship travels toward destination");
            require(!trip.selectDestination(0), "cannot change route mid-flight");
            trip.update(9);
            require(trip.flight == (destinations[i].dangerous ? Flight::Destroyed : Flight::Arrived), "route resolves to correct outcome");
            if (trip.canLand()) {
                require(std::abs(glm::length(trip.position()-destinations[i].center) - destinations[i].radius - .15f)<.002f,
                        "touchdown rests on the selected surface");
                glm::vec3 up = glm::normalize(glm::vec3(trip.rocket()[1]));
                require(glm::dot(up,trip.landingNormal())>.999f, "landed rocket stands upright");
            }
            require(!trip.ignition(), "engines stop at terminal outcome");
            require(!trip.resultReady(), "outcome is not announced immediately");
            trip.speed=20;
            trip.update(2);
            require(!trip.resultReady(), "20x playback does not skip result delay");
            trip.paused=true; trip.update(20);
            require(!trip.resultReady(), "pause freezes result delay");
            trip.paused=false; trip.update(3);
            require(trip.resultReady(), "result appears after five real seconds");
            auto end = trip.position(); trip.update(100);
            require(glm::length(trip.position()-end) == 0, "terminal position stays bounded");
        }
        trip.reset();
        require(trip.destination == -1 && trip.aftermath == 0 && trip.flight == Flight::Ascent, "restart clears space mission");
        return 0;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
