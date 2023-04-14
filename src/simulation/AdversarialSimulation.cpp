#include "AdversarialSimulation.h"
#include <algorithm>

AdversarialSimulation::AdversarialSimulation() {}

void AdversarialSimulation::startParallelGeneration(bool oneRealtime) {
    // The adversarial does not strictly need to have the simulation count match the pool size
    BaseSimulation::startParallelGeneration(oneRealtime, m_poolSize);
}

bool AdversarialSimulation::preSimulationFrame(Simulation* simulation) {
    size_t frame = simulation->getFrameNumber();
    if (frame == m_framesPerSimulation && !simulation->isMarkedAsFinished()) { // The simulation is now done
        simulation->markAsFinished(); // Stores the scores of all cars currently driving
        m_simulationsLeft.fetch_sub(1, std::memory_order::memory_order_release);
    }

    for (auto it =  m_carSpawnTimes.find(frame);
         it != m_carSpawnTimes.end() && it->first == frame; ++it) {
        //size_t index = simulation->getIndexInGeneration() + simulation->getNumberOfSpawnedCars();
        size_t index = (simulation->getNumberOfSpawnedCars() % 5 == 0) ? 0 : simulation->getIndexInGeneration();
        simulation->spawnCar(it->second, &m_geneticPool[index % m_poolSize], m_spawnRandomness);
    }

    return !simulation->isMarkedAsFinished();
}

void AdversarialSimulation::evolveGenePool() {
    std::vector<std::pair<float, int>> scores;
    for (size_t i = 0; i < m_poolSize; i++) {
        CarBrain* brain = &m_geneticPool[i];
        float scoreSum = 0.f;
        size_t count = 0;
        for (auto& car_score : m_simulations[i].getFinalCarScores())
            if (car_score.first == brain) {
                scoreSum += car_score.second;
                count++;
            }
        scores.emplace_back(scoreSum / count, i);
    }

    std::sort(scores.rbegin(), scores.rend());

    printBrainScores(scores);

    geneticEvolveGenePool(scores);
}