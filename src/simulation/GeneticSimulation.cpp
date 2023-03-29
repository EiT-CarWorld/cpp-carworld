#include "GeneticSimulation.h"
#include <cassert>
#include <iostream>

GeneticSimulation::GeneticSimulation(std::vector<CarBrain> initial_brains) : BaseSimulation(std::move(initial_brains)) {}

bool GeneticSimulation::handleOption(std::string &option, std::ifstream &file, bool ignoreSaveLoad) {


    // Fallback to options defined by BaseSimulation
    return BaseSimulation::handleOption(option, file, ignoreSaveLoad);
}

void GeneticSimulation::startParallelGeneration(bool oneRealtime) {
    // In case randomized route picking is enabled, calculate them now
    m_routesPicker.updateRoutePicks(m_generation, m_seed, m_world.getRoutes(), m_carSpawnTimes);

    BaseSimulation::startParallelGeneration(oneRealtime);
}

bool GeneticSimulation::preSimulationFrame(Simulation* simulation) {
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