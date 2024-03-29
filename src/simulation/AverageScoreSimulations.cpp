#include "AverageScoreSimulations.h"
#include <algorithm>
#include <iostream>

AverageScoreSimulations::AverageScoreSimulations() {}

void AverageScoreSimulations::startParallelGeneration(bool oneRealtime) {
    BaseSimulation::startParallelGeneration(oneRealtime, m_poolSize);
}

bool AverageScoreSimulations::preSimulationFrame(Simulation* simulation) {
    size_t frame = simulation->getFrameNumber();
    if (frame == m_framesPerSimulation && !simulation->isMarkedAsFinished()) { // The simulation is now done
        simulation->markAsFinished(); // Stores the scores of all cars currently driving
        m_simulationsLeft.fetch_sub(1, std::memory_order::memory_order_release);
    }

    for (auto it =  m_carSpawnTimes.find(frame);
         it != m_carSpawnTimes.end() && it->first == frame; ++it) {
        //size_t index = simulation->getIndexInGeneration() + simulation->getNumberOfSpawnedCars();
        //size_t index = (simulation->getNumberOfSpawnedCars() % 5 == 0) ? 0 : simulation->getIndexInGeneration();

        // We have removed the "adversarial" part of the simulation, all cars get the same brain again
        size_t index = simulation->getIndexInGeneration();
        simulation->spawnCar(it->second, &m_geneticPool[index], m_spawnRandomness);
    }

    return !simulation->isMarkedAsFinished();
}

void AverageScoreSimulations::evolveGenePool() {
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

        //double dead_cars_multiplier = std::pow(EACH_DEAD_CAR_MULTIPLIER, m_simulations[i].getNumberOfDeadCars());
        scores.emplace_back(scoreSum / count, i);
    }

    std::sort(scores.rbegin(), scores.rend());

    printBrainScores(scores);

    int bestBrain = scores.begin()->second;
    std::cout << "Generation " << m_generation << ": ";
    m_simulations[bestBrain].printSummary();

    geneticEvolveGenePool(scores);
}