#include "GeneticSimulation.h"
#include <cassert>
#include <algorithm>
#include <iostream>
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

void GeneticSimulation::loadParameterFile(const char* path) {
    assert(!hasGenerationRunning()); // We can't change parameters during execution

    std::ifstream file;
    file.open(path);
    if (file.fail()) {
        std::cerr << "error: opening parameter file '" << path << "'" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cerr << "info: loading parameters from '" << path << "'" << std::endl;

    std::string option;
    while(true) {
        file >> option;
        if (file.eof())
            break;
        if (option == "world") {
            std::string filepath;
            file >> filepath;
            m_world.loadFromFile(filepath);
            assert(m_seed);
            m_world.createRoutes(m_seed, 2);
        } else if(option == "spawnTimes") {
            m_carSpawnTimes.clear();
            size_t count;
            file >> count;
            for (int i = 0; i < count; i++) {
                assert (file.good() && file.get() == '\n');
                size_t frame, route;
                file >> frame >> route;
                m_carSpawnTimes.insert({frame, route});
            }
        } else if (option == "seed")
            file >> m_seed;
        else if (option == "poolSize")
            file >> m_poolSize;
        else if (option == "survivorsPerGeneration")
            file >> m_survivorsPerGeneration;
        else if (option == "framesPerSimulation")
            file >> m_framesPerSimulation;
        else {
            std::cerr << "error: unknown parameter '" << option << "'" << std::endl;
            break;
        }

        if (file.get() != '\n') {
            std::cerr << "error: expected newline after option '" << option << "'" << std::endl;
            break;
        }
        if (file.fail()) {
            std::cerr << "error: setting the parameter '" << option << "' failed" << std::endl;
            break;
        }
    }
    file.close();

    // Do a bunch of asserts to make sure the state is legal
    assert (m_world.isLoaded());
    assert (m_seed);
    assert (m_poolSize);
    assert (m_survivorsPerGeneration);
    assert (m_framesPerSimulation);
}

void GeneticSimulation::setScoreOutputFile(const char* path) {
    if (path) {
        if (m_brainScoreOutput.is_open())
            m_brainScoreOutput.close();

        m_brainScoreOutput.open(path);
        if (m_brainScoreOutput.fail()) {
            std::cerr << "warning: opening file for brain score output failed: " << path << std::endl;
        }
    } else {
        m_brainScoreOutput.close();
    }
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
    // Having a world serves as a sentinel for being initialized
    assert (m_world.isLoaded());

    m_hasRealtimeSimulation = oneRealtime;
    fillGenePool(); // Creates enough brains to do poolSize simulations

    // Creates one simulation per brain, with identical seed and world
    for (auto& brain:m_geneticPool)
        m_simulations.emplace_back(&m_world, &brain, m_seed, false);
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

    for (auto it =  m_carSpawnTimes.find(frame);
         it != m_carSpawnTimes.end() && it->first == frame; ++it)
        simulation->spawnCar(it->second);
    return true;
}

// Gives the number of simulations that have yet to finish in the generation
size_t GeneticSimulation::getSimulationsRunning() {
    return m_simulationsLeft.load(std::memory_order_acquire);
}

void GeneticSimulation::pruneGenePool() {
    // Only keep the best brains. Sort their scores to find the cutoff
    std::vector<float> scores;
    scores.reserve(m_poolSize);
    for (auto& brain:m_geneticPool)
        scores.push_back(brain.getEvaluationScore());
    // Put the largest scores first
    std::sort(scores.rbegin(), scores.rend());

    // If we have a file for printing brain scores open, print them all there
    if (m_brainScoreOutput.is_open()) {
        m_brainScoreOutput << m_generation;
        for(float score : scores) {
            m_brainScoreOutput << "," << score;
        }
        m_brainScoreOutput << std::endl;
    }

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
}

void GeneticSimulation::finishGeneration() {
    assert (hasGenerationRunning() && getSimulationsRunning() == 0);

    // All threads should be done by now, but join up for good measure
    for (auto& thread:m_threads)
        thread.join();
    m_threads.clear();

    m_simulations.clear(); // The scores are stored in the brains, so the simulations are longer needed

    pruneGenePool();

    m_generation++;
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
    m_hasRealtimeSimulation = false;

    m_isGenerationAborted.store(false, std::memory_order_release);
}

