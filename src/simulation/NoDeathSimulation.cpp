#include "NoDeathSimulation.h"
#include <cassert>
#include <algorithm>

NoDeathSimulation::NoDeathSimulation() {}

void NoDeathSimulation::startParallelGeneration(bool oneRealtime) {
    BaseSimulation::startParallelGeneration(oneRealtime, m_poolSize);
}

bool NoDeathSimulation::preSimulationFrame(Simulation* simulation) {
    size_t frame = simulation->getFrameNumber();
    if ((frame == m_framesPerSimulation || simulation->hasCarDied()) && !simulation->isMarkedAsFinished()) { // The simulation is now done
        simulation->markAsFinished(); // Stores the scores of all cars currently driving
        m_simulationsLeft.fetch_sub(1, std::memory_order::memory_order_release);
    }

    for (auto it =  m_carSpawnTimes.find(frame);
         it != m_carSpawnTimes.end() && it->first == frame; ++it) {
        simulation->spawnCar(it->second, &m_geneticPool[simulation->getIndexInGeneration()], m_spawnRandomness);
    }

    return !simulation->isMarkedAsFinished();
}

void NoDeathSimulation::evolveGenePool() {
    assert(m_simulations.size() == m_poolSize);

    std::vector<std::pair<float, int>> scores;
    for (size_t i = 0; i < m_poolSize; i++)
        scores.emplace_back(m_simulations[i].getTotalSimulationScore(), i);

    std::sort(scores.rbegin(), scores.rend());

    printBrainScores(scores);

    geneticEvolveGenePool(scores);
}