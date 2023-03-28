#include "GeneticSimulation.h"
#include <cassert>
#include <iostream>

GeneticSimulation::GeneticSimulation(std::vector<CarBrain> initial_brains) : BaseSimulation(std::move(initial_brains)) {}

bool GeneticSimulation::handleOption(std::string &option, std::ifstream &file, bool ignoreSaveLoad) {
    if (option == "defineRoutes") {
        m_carSpawnTimes.clear();
        m_world.clearRoutes();
        size_t count;
        file >> count;
        for (int i = 0; i < count; i++) {
            OR_COMPLAIN(file.good() && file.get() == '\n');
            size_t u, v;
            file >> u >> v;
            OR_COMPLAIN(u >= 0 && u < m_world.getNodes().size());
            OR_COMPLAIN(v >= 0 && v < m_world.getNodes().size());
            m_world.addRoute(u, v);
        }
        return true;
    }

    if (option == "spawnTimes") {
        m_routesPicker.stopRandomRoutePicking();
        m_carSpawnTimes.clear();
        size_t count;
        file >> count;
        for (int i = 0; i < count; i++) {
            OR_COMPLAIN(file.good() && file.get() == '\n');
            size_t frame, route;
            file >> frame >> route;
            OR_COMPLAIN(route >= 0 && route < m_world.getRoutes().size());
            m_carSpawnTimes.insert({frame, route});
        }
        return true;
    }

    if (option == "pickRandomRoutes") {
        int period, spawnFramePeriod, minDelay, maxDelay, lastSpawnableFrame;
        file >> period >> spawnFramePeriod >> minDelay >> maxDelay >> lastSpawnableFrame;
        m_routesPicker.startRandomRoutePicking(period, spawnFramePeriod, minDelay, maxDelay, lastSpawnableFrame);
        return true;
    }

    // Fallback to options defined by BaseSimulation
    return BaseSimulation::handleOption(option, file, ignoreSaveLoad);
}

void GeneticSimulation::startParallelGeneration(bool oneRealtime) {
    // In case randomized route picking is enabled, calculate them now
    m_routesPicker.updateRoutePicks(m_generation, m_seed, m_world.getRoutes(), m_carSpawnTimes);

    BaseSimulation::startParallelGeneration(oneRealtime);
}

bool GeneticSimulation::preSimulationFrame(Simulation* simulation) {
    if (simulation->isMarkedAsFinished())
        return false;

    size_t frame = simulation->getFrameNumber();
    if (frame == m_framesPerSimulation || simulation->hasCarDied()) { // The simulation is now done
        simulation->markAsFinished(); // Stores the current score into the brain
        m_simulationsLeft.fetch_sub(1, std::memory_order::memory_order_release);
        return false;
    }

    for (auto it =  m_carSpawnTimes.find(frame);
         it != m_carSpawnTimes.end() && it->first == frame; ++it) {
        simulation->spawnCar(it->second, m_spawnRandomness);
    }
    return true;
}