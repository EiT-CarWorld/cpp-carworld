#include "BaseSimulation.h"
#include <cassert>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "carConfig.h"
#include "util.h"

BaseSimulation::BaseSimulation(std::vector<CarBrain> initial_brains) : m_geneticPool(std::move(initial_brains)) {
    assert(!m_geneticPool.empty());
}

size_t BaseSimulation::getGenerationNumber() {
    return m_generation;
}

size_t BaseSimulation::getFramesPerSimulation() {
    return m_framesPerSimulation;
}

bool BaseSimulation::handleOption(std::string &option, std::ifstream &file, bool ignoreSaveLoad) {
    if (option == "world") {
        std::string filepath;
        file >> filepath;
        return m_world.loadFromFile(filepath );
    }

    if (option == "saveGeneration") {
        std::string dest;
        file >> dest;
        if (!ignoreSaveLoad)
            OR_COMPLAIN(saveGenePool(dest.c_str()));
        return true;
    }

    if (option == "loadGeneration") {
        std::string src;
        file >> src;
        if (!ignoreSaveLoad)
            OR_COMPLAIN(loadGenePool(src.c_str()));
        return true;
    }

    if (option == "seed") {
        file >> m_seed;
        return true;
    }

    if (option == "poolSize") {
        file >> m_poolSize;
        return true;
    }

    if (option == "survivorsPerGeneration") {
        file >> m_survivorsPerGeneration;
        return true;
    }

    if (option == "framesPerSimulation") {
        file >> m_framesPerSimulation;
        return true;
    }

    if (option == "mutationChance") {
        file >> m_mutationChance;
        return true;
    }

    if (option == "spawnRandomness") {
        file >> m_spawnRandomness;
        return true;
    }

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

    std::cerr << "error: unknown parameter '" << option << "'" << std::endl;
    return false;
}

bool BaseSimulation::loadParameterFile(const char* path, bool ignoreSaveLoad) {
    assert(!hasGenerationRunning()); // We can't change parameters during execution

    std::ifstream file;
    file.open(path);
    if (file.fail()) {
        std::cerr << "error: opening parameter file '" << path << "'" << std::endl;
        return false;
    }

    TraceLog(LOG_INFO, "loading parameters from %s", path);

    std::string option;
    while(true) {
        // If the file starts with #
        if (file.peek() == '#') {
            // Skip this line
            while(file.get() != '\n' && file.good());
            continue;
        }

        file >> option;
        if (file.eof())
            break;

        if (!handleOption(option, file, ignoreSaveLoad))
            break;

        if (file.get() != '\n') {
            std::cerr << "error: expected newline after option '" << option << "'" << std::endl;
            return false;
        }
        if (file.fail()) {
            std::cerr << "error: setting the parameter '" << option << "' failed" << std::endl;
            return false;
        }
    }
    file.close();

    // Do a bunch of asserts to make sure the state is legal
    OR_COMPLAIN (m_world.isLoaded());
    OR_COMPLAIN (m_seed);
    OR_COMPLAIN (m_poolSize);
    OR_COMPLAIN (m_survivorsPerGeneration);
    OR_COMPLAIN (m_framesPerSimulation);

    return true;
}

bool BaseSimulation::saveGenePool(const char *path) {
    assert(!hasGenerationRunning());
    TraceLog(LOG_INFO, "Saving generation to file '%s'", path);

    std::ofstream brainSave;
    brainSave.open(path);
    OR_COMPLAIN(brainSave.good());

    brainSave << m_generation << std::endl;
    brainSave << m_geneticPool.size() << std::endl;
    for ( CarBrain& brain : m_geneticPool )
        brain.saveToFile(brainSave);
    return true;
}

bool BaseSimulation::loadGenePool(const char *path) {
    assert(!hasGenerationRunning());
    TraceLog(LOG_INFO, "Loading generation from file '%s'", path);

    std::ifstream brainLoad;
    brainLoad.open(path);
    OR_COMPLAIN(brainLoad.good());

    brainLoad >> m_generation;
    OR_COMPLAIN(brainLoad.good() && brainLoad.get() == '\n');

    size_t num_brains;
    brainLoad >> num_brains;
    OR_COMPLAIN(brainLoad.good() && brainLoad.get() == '\n');
    m_geneticPool.clear();
    for (size_t i = 0; i < num_brains; i++)
        m_geneticPool.emplace_back(CarBrain::loadFromFile(brainLoad));
    return true;
}

bool BaseSimulation::loadParameterFileIfExists(const char *path, bool ignoreSaveLoad) {
    if ( !fileExists(path) )
        return true;
    return loadParameterFile(path, ignoreSaveLoad);
}

void BaseSimulation::setScoreOutputFile(const char* path) {
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

void BaseSimulation::fillGenePool() {
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

        brain.mutate(prng, m_mutationChance);
        m_geneticPool.emplace_back(std::move(brain));
    }
}

void BaseSimulation::runSimulationsInThread(size_t begin, size_t end) {
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

bool BaseSimulation::hasGenerationRunning() {
    return !m_simulations.empty();
}

void BaseSimulation::startParallelGeneration(bool oneRealtime) {
    // The last generation must be done
    assert (!hasGenerationRunning());
    assert (m_threads.empty());
    // Having a world serves as a sentinel for being initialized
    assert (m_world.isLoaded());

    m_hasRealtimeSimulation = oneRealtime;
    fillGenePool(); // Creates enough brains to do poolSize simulations

    // Creates one simulation per brain, with identical seed and world
    for (auto& brain:m_geneticPool)
        m_simulations.emplace_back(&m_world, &brain, m_seed+m_generation, false);
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

Simulation* BaseSimulation::getRealtimeSimulation() {
    if (!m_hasRealtimeSimulation)
        return nullptr;
    return &m_simulations.front();
}

// Gives the number of simulations that have yet to finish in the generation
size_t BaseSimulation::getSimulationsRunning() {
    return m_simulationsLeft.load(std::memory_order_acquire);
}

void BaseSimulation::pruneGenePool() {
    // Only keep the best brains. Sort their scores to find the cutoff
    std::vector<std::pair<float, int>> scores;
    scores.reserve(m_poolSize);
    for (size_t i = 0; i < m_geneticPool.size(); i++)
        scores.emplace_back(m_geneticPool[i].getEvaluationScore(), i);
    // Put the largest scores first
    std::sort(scores.rbegin(), scores.rend());

    // If we have a file for printing brain scores open, print them all there
    if (m_brainScoreOutput.is_open()) {
        if (m_generation == 0) {
            m_brainScoreOutput << "gen";
            for ( size_t i = 0; i < scores.size(); i++ )
                m_brainScoreOutput << ",brain" << (i+1);
            m_brainScoreOutput << std::endl;
        }
        m_brainScoreOutput << m_generation;
        for(auto score : scores)
            m_brainScoreOutput << "," << std::fixed << std::setprecision(3) << score.first;
        m_brainScoreOutput << std::endl;
    }

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

void BaseSimulation::finishGeneration() {
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

void BaseSimulation::abortGeneration() {
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