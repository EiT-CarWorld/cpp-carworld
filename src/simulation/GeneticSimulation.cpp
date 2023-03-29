#include "GeneticSimulation.h"
#include <cassert>
#include <iostream>
#include <algorithm>

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
    if ((frame == m_framesPerSimulation || simulation->hasCarDied()) && !simulation->isMarkedAsFinished()) { // The simulation is now done
        simulation->markAsFinished(); // Stores the current score into the brain
        m_simulationsLeft.fetch_sub(1, std::memory_order::memory_order_release);
        return false;
    }

    for (auto it =  m_carSpawnTimes.find(frame);
         it != m_carSpawnTimes.end() && it->first == frame; ++it) {
        simulation->spawnCar(it->second, &m_geneticPool[simulation->getIndexInGeneration()], m_spawnRandomness);
    }
    return true;
}

void GeneticSimulation::pruneGenePool() {
    assert (m_simulations.size() == m_geneticPool.size());

    // Only keep the best brains. Sort their scores to find the cutoff
    std::vector<std::pair<float, int>> scores;
    scores.reserve(m_poolSize);
    for (size_t i = 0; i < m_simulations.size(); i++)
        scores.emplace_back(m_simulations[i].getTotalSimulationScore(), i);
    // Put the largest scores first
    std::sort(scores.rbegin(), scores.rend());

    printBrainScores (scores);

    if (m_survivorsPerGeneration == m_poolSize)
        return;

    // Only keep the top performers, in the new gene pool
    std::vector<CarBrain> newGenePool;
    for (size_t i = 0; i < m_survivorsPerGeneration; i++) {
        CarBrain&& brain = std::move(m_geneticPool[scores[i].second]);
        newGenePool.emplace_back(brain);
    }

    m_geneticPool.swap(newGenePool);
}