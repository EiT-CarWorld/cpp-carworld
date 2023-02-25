#include "GeneticSimulation.h"
#include <cassert>
#include <algorithm>
#include "entities/World.h"
#include "carConfig.h"

GeneticSimulation::GeneticSimulation(World *world, unsigned long seed, std::unordered_map<size_t, size_t> carSpawnTimes,
                                     std::vector<CarBrain> initial_brains, size_t poolSize,
                                     size_t survivorsPerGeneration, size_t framesPerSimulation) :
                                     m_world(world), m_seed(seed), m_carSpawnTimes(std::move(carSpawnTimes)), m_geneticPool(std::move(initial_brains)),
                                     m_poolSize(poolSize), m_survivorsPerGeneration(survivorsPerGeneration), m_framesPerSimulation(framesPerSimulation),
                                     m_generation(0) {
    assert(!m_geneticPool.empty());
    assert(m_poolSize && m_survivorsPerGeneration && m_framesPerSimulation);
    assert(m_survivorsPerGeneration < m_poolSize);
}

size_t GeneticSimulation::getGenerationNumber() {
    return m_generation;
}

size_t GeneticSimulation::getFramesPerSimulation() {
    return m_framesPerSimulation;
}

void GeneticSimulation::fillGenePool() {
    assert (!m_geneticPool.empty());

    while (m_geneticPool.size() < m_poolSize) {
        m_geneticPool.push_back(m_geneticPool[0]);
        //TODO: Make changes
    }
}

void GeneticSimulation::runSimulationsInThread(size_t begin, size_t end) {
    for(size_t i = begin; i < end && !m_isAborted; i++) {
        auto& simulation = m_simulations[i];
        while (preSimulationFrame(&simulation)) {
            simulation.takeCarActions();
            simulation.updateCars();
        }
    }
}

void GeneticSimulation::startParallelGeneration(bool oneRealtime) {
    // Aborted generations can not be restarted
    assert (!m_isAborted);
    // The last generation must be done
    assert (m_simulations.empty());
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
        std::thread thread([=]() { this->runSimulationsInThread(begin, end); });
        m_threads.emplace_back(std::move(thread));
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

size_t GeneticSimulation::getCurrentlyRunning() {
    return m_simulationsLeft.load(std::memory_order_acquire);
}

void GeneticSimulation::finishGeneration() {
    assert (getCurrentlyRunning() == 0);
    assert (!m_isAborted);

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

    // Remove all brains at or below this limit
    float removal_limit = scores[m_survivorsPerGeneration];
    m_geneticPool.erase(std::remove_if(m_geneticPool.begin(), m_geneticPool.end(), [=](CarBrain& it) {
        return it.getEvaluationScore() <= removal_limit;
    }), m_geneticPool.end());
    // Make sure we kept at least one
    assert(!m_geneticPool.empty());

    m_hasRealtimeSimulation = false;
}

void GeneticSimulation::abortGeneration() {
    m_isAborted = true;
    for (auto& thread:m_threads)
        thread.join();
    m_threads.clear();
}