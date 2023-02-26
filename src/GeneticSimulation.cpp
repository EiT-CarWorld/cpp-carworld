#include "GeneticSimulation.h"
#include <cassert>
#include <algorithm>
#include "entities/World.h"
#include "carConfig.h"

GeneticSimulation::GeneticSimulation(std::vector<CarBrain> initial_brains) : m_geneticPool(std::move(initial_brains)) {
    assert(!m_geneticPool.empty());
}

size_t GeneticSimulation::getGenerationNumber() {
    return m_generation;
}

size_t GeneticSimulation::getFramesPerSimulation() {
    return m_framesPerSimulation;
}

void GeneticSimulation::fillGenePool() {
    assert(!m_geneticPool.empty());
    m_parentsThisGeneration = m_geneticPool.size();
    assert(m_parentsThisGeneration <= m_poolSize);
    std::uniform_int_distribution<size_t> parentSelect(0, m_parentsThisGeneration-1);

    std::mt19937 prng (m_seed + m_generation);

    while (m_geneticPool.size() < m_poolSize) {
        size_t parent1 = parentSelect(prng);
        size_t parent2 = parentSelect(prng);

        CarBrain brain = m_geneticPool[parent1];
        if(parent1 != parent2)
            brain.mixIn(m_geneticPool[parent2], prng);

        brain.mutate(prng);
        m_geneticPool.emplace_back(std::move(brain));
    }
}

void GeneticSimulation::runSimulationsInThread(size_t begin, size_t end) {
    m_threads.emplace_back([=]() {
        for (size_t i = begin; i < end && !m_isGenerationAborted.load(std::memory_order_relaxed); i++) {
            auto &simulation = m_simulations[i];
            while (preSimulationFrame(&simulation)) {
                simulation.takeCarActions();
                simulation.updateCars();
            }
        }
    });
}

bool GeneticSimulation::hasGenerationRunning() {
    return !m_simulations.empty();
}

void GeneticSimulation::startParallelGeneration(bool oneRealtime) {
    // The last generation must be done
    assert (!hasGenerationRunning());
    assert (m_threads.empty());

    m_hasRealtimeSimulation = oneRealtime;
    m_generation++;
    fillGenePool(); // Creates enough brains to do poolSize simulations

    // Creates one simulation per brain, with identical seed and world
    for (auto& brain:m_geneticPool)
        m_simulations.emplace_back(m_world, &brain, m_seed, false);
    assert(m_simulations.size() == m_poolSize);
    m_simulationsLeft.store(m_poolSize);

    // Partition up the simulations among the threads
    size_t start = oneRealtime ? 1 : 0;
    size_t count = m_poolSize - start;
    for(int i = 0; i < THREAD_COUNT; i++) {
        size_t begin = start+(count*i/THREAD_COUNT);
        size_t end = start+(count*(i+1)/THREAD_COUNT);
        runSimulationsInThread(begin, end);
    }
}

Simulation* GeneticSimulation::getRealtimeSimulation() {
    if (!m_hasRealtimeSimulation)
        return nullptr;
    return &m_simulations.front();
}

bool GeneticSimulation::preSimulationFrame(Simulation* simulation) {
    size_t frame = simulation->getFrameNumber();

    if (frame == m_framesPerSimulation) { // The simulation is now done
        simulation->storeTotalScoreInBrain(); // Assign the score it has, to the brain, for later sorting
        m_simulationsLeft.fetch_sub(1, std::memory_order::memory_order_release);
        return false;
    }
    if (frame > m_framesPerSimulation)
        return false;

    auto it = m_carSpawnTimes.find(frame);
    if (it != m_carSpawnTimes.end())
        for (int i = 0; i < it->second; i++)
            simulation->spawnCar();
    return true;
}

// Gives the number of simulations that have yet to finish in the generation
size_t GeneticSimulation::getSimulationsRunning() {
    return m_simulationsLeft.load(std::memory_order_acquire);
}

void GeneticSimulation::finishGeneration() {
    assert (hasGenerationRunning() && getSimulationsRunning() == 0);

    // All threads should be done by now, but join up for good measure
    for (auto& thread:m_threads)
        thread.join();
    m_threads.clear();

    m_simulations.clear(); // The scores are stored in the brains, so the simulations are longer needed

    // Only keep the best brains. Sort their scores to find the cutoff
    std::vector<float> scores;
    scores.reserve(m_poolSize);
    for (auto& brain:m_geneticPool)
        scores.push_back(brain.getEvaluationScore());
    // Put the largest scores first
    std::sort(scores.rbegin(), scores.rend());

    // Keep all brains above, and one equal to this score
    float removal_limit = scores[m_survivorsPerGeneration];
    bool keptOne = false;
    m_geneticPool.erase(std::remove_if(m_geneticPool.begin(), m_geneticPool.end(), [&](CarBrain& it) {
        if (it.getEvaluationScore() == removal_limit && !keptOne) {
            keptOne = true;
            return false;
        }
        return it.getEvaluationScore() <= removal_limit;
    }), m_geneticPool.end());

    m_hasRealtimeSimulation = false;
}

void GeneticSimulation::abortGeneration() {
    if (!hasGenerationRunning())
        return;

    m_isGenerationAborted.store(true, std::memory_order_release); // Makes the threads finish quicker
    for (auto& thread:m_threads)
        thread.join();
    m_threads.clear();

    // Remove all brains except those who were survivors from the previous finished generation
    m_geneticPool.erase(m_geneticPool.begin() + m_parentsThisGeneration, m_geneticPool.end());

    // Remove all signs of the generation running, or having ever ran
    m_simulations.clear();
    m_simulationsLeft.store(0);
    m_generation--;

    m_isGenerationAborted.store(false, std::memory_order_release);
}

