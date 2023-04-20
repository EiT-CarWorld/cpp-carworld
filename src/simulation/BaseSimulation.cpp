#include "BaseSimulation.h"
#include <cassert>
#include <algorithm>
#include <iostream>
#include "carConfig.h"
#include "util.h"
#include <iomanip>

BaseSimulation::BaseSimulation() {}

size_t BaseSimulation::getGenerationNumber() {
    return m_generation;
}

size_t BaseSimulation::getFramesPerSimulation() {
    return m_framesPerSimulation;
}

CarBrain* BaseSimulation::getBestBrain() {
    assert(!m_geneticPool.empty());
    return &m_geneticPool[0];
}

bool BaseSimulation::handleOption(std::string &option, std::ifstream &file, bool ignore_gene_pool) {
    if (option == "saveGeneration") {
        std::string dest;
        file >> dest;
        if (!ignore_gene_pool)
            OR_COMPLAIN(saveGenePool(dest.c_str()));
        return true;
    }

    if (option == "loadGeneration") {
        std::string src;
        file >> src;
        if (!ignore_gene_pool)
            OR_COMPLAIN(loadGenePool(src.c_str()));
        return true;
    }

    if (option == "brainLayers") {
        size_t layers;
        file >> layers;
        // We can not change the layers in the brain once set
        OR_COMPLAIN(ignore_gene_pool || m_brain_layers.empty());
        for (size_t i = 0; i < layers; i++) {
            size_t layer_size;
            file >> layer_size;
            //if (!ignore_gene_pool)
            m_brain_layers.push_back(layer_size);
        }

        return true;
    }

    if (option == "poolSize") {
        file >> m_poolSize;
        return true;
    }

    if (option == "world") {
        std::string filepath;
        file >> filepath;
        OR_RETURN(m_world.loadFromFile(filepath ));
        m_carSpawnTimes.clear();
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

    if (option == "seed") {
        file >> m_seed;
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
        // If the file starts with #, or is blank
        if (file.peek() == '#' || file.peek() == '\n') {
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
    OR_COMPLAIN(m_world.isLoaded());
    OR_COMPLAIN(!m_brain_layers.empty());
    OR_COMPLAIN(m_seed);
    OR_COMPLAIN(m_poolSize >= 1);
    OR_COMPLAIN(m_framesPerSimulation >= 1);

    return true;
}

bool BaseSimulation::saveGenePool(const char *path) {
    assert(!hasGenerationRunning());
    TraceLog(LOG_INFO, "Saving generation to file '%s'", path);

    std::ofstream brainSave;
    brainSave.open(path, std::ios::binary);
    OR_COMPLAIN(brainSave.good());

    WRITEOUT(brainSave, m_generation);
    WRITEOUT(brainSave, (size_t)m_brain_layers.size());
    for ( size_t layerSize: m_brain_layers )
        WRITEOUT(brainSave, layerSize);

    WRITEOUT(brainSave, (size_t)m_geneticPool.size());
    for ( CarBrain& brain : m_geneticPool )
        brain.saveToFile(brainSave);
    return true;
}

bool BaseSimulation::loadGenePool(const char *path) {
    assert(!hasGenerationRunning());
    TraceLog(LOG_INFO, "Loading generation from file '%s'", path);

    std::ifstream brainLoad;
    brainLoad.open(path, std::ios::binary);
    OR_COMPLAIN(brainLoad.good());

    READIN(brainLoad, m_generation);

    // Read in the definition of layers used in all the brains
    size_t num_brain_layers;
    READIN(brainLoad, num_brain_layers);
    m_brain_layers.resize(num_brain_layers);
    for ( size_t i = 0; i < num_brain_layers; i++ )
        READIN(brainLoad, m_brain_layers[i]);

    size_t num_brains;
    READIN(brainLoad, num_brains);
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

void BaseSimulation::fitGenePoolToSize() {
    assert(!m_brain_layers.empty());
    assert(m_poolSize >= 1);

    while (m_geneticPool.size() < m_poolSize) {
        auto matrices = CarBrain::initializeMatrices(m_seed + m_geneticPool.size(),  m_brain_layers);
        m_geneticPool.emplace_back(std::move(matrices));
    }

    if (m_geneticPool.size() > m_poolSize)
        m_geneticPool.erase(m_geneticPool.begin() + m_poolSize, m_geneticPool.end());
}

void BaseSimulation::printBrainScores(const std::vector<std::pair<float, int>> &scores) {
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
}

#define NUM_PARENTS 20
// Uses the provided (score, brain) pairs to create a new generation
void BaseSimulation::geneticEvolveGenePool(std::vector<std::pair<float, int>> const& scores) {
    assert(m_geneticPool.size() == m_poolSize);
    assert(scores.size() == m_poolSize);

    std::vector<CarBrain> newPool;

    // First of all, save the brain with the best score ever seen
    float best_ever_score = 0.0f;
    size_t best_ever_brain = 0;
    for (size_t i = 0; i < m_poolSize; i++) {
        m_geneticPool[scores[i].second].informAboutScoreAchieved(scores[i].first);
        float brains_best = m_geneticPool[scores[i].second].getBestScoreAchieved();
        if (brains_best > best_ever_score) {
            best_ever_score = brains_best;
            best_ever_brain = scores[i].second;
        }
    }
    newPool.push_back(m_geneticPool[best_ever_brain]);

    // Save the elite
    for (size_t i = 0; i < m_survivorsPerGeneration; i++) {
        if (scores[i].second == best_ever_brain)
            continue;
        newPool.emplace_back(m_geneticPool[scores[i].second]);
    }

    // Calculate average score to compare all scores against
    float average_score = 0.f;
    for (auto s:scores) {
        average_score += s.first / scores.size();
    }

    // Make a PRNG to perform evolution
    std::mt19937 rand(m_seed + m_generation);

    // for the rest, pick k parents at random, weighted by distance to average score
    float scoreSum = 0.f;
    for (auto& it : scores)
        scoreSum += std::fmaxf(it.first - average_score, 0.f);
    std::uniform_real_distribution<float> parentChoiceDist(0.f, scoreSum);

    std::vector<CarBrain*> parents;
    for (size_t i = 0; i < NUM_PARENTS; i++) {
        float choiceScore = parentChoiceDist(rand);
        size_t parent = 0;
        while (parent < m_poolSize && choiceScore > 0) {
            choiceScore -= std::fmaxf(scores[parent].first - average_score, 0.f);
            parent++;
        }
        parent--; // Go back one step
        parents.push_back(&m_geneticPool[scores[parent].second]);
    }

    // Now go through and mix parents pairwise, in a loop
    for (size_t i = 0; newPool.size() < m_poolSize; i++) {
        CarBrain* p1 = parents[i % parents.size()];
        CarBrain* p2 = parents[(i+1) % parents.size()];

        // Each child is given exactly the opposite selection
        CarBrain child1 = *p1, child2 = *p2;
        std::mt19937 rand1(m_seed+m_generation+i), rand2(m_seed+m_generation+i);
        child1.mixIn(*p2, rand1);
        child2.mixIn(*p1, rand2);
        child1.mutate(rand1, m_mutationChance);
        child2.mutate(rand2, m_mutationChance);

        newPool.emplace_back(std::move(child1));
        newPool.emplace_back(std::move(child2));
    }

    // In case we added too many
    while (newPool.size() > m_poolSize)
        newPool.pop_back();

    // Insert the new pool in place of the old
    std::swap(m_geneticPool, newPool);
}

void BaseSimulation::runSimulationsInThread(size_t offset, size_t stride, size_t end) {
    m_threads.emplace_back([=]() {
        for (size_t i = offset; i < end && !m_isGenerationAborted.load(std::memory_order_relaxed); i+=stride) {
            while (preSimulationFrame(&m_simulations[i])) {
                m_simulations[i].takeCarActions();
                m_simulations[i].updateCars();
            }
        }
    });
}

bool BaseSimulation::hasGenerationRunning() {
    return !m_simulations.empty();
}

void BaseSimulation::startParallelGeneration(bool oneRealtime, size_t simulation_count) {
    // The last generation must be done
    assert (!hasGenerationRunning());
    assert (m_threads.empty());
    // Having a world serves as a sentinel for being initialized
    assert (m_world.isLoaded());

    m_hasRealtimeSimulation = oneRealtime;

    m_routesPicker.updateRoutePicks(m_generation, m_seed, m_world.getRoutes(), m_carSpawnTimes);

    // In case the gene pool is not currently the correct size
    fitGenePoolToSize();

    // Creates one simulation per brain, with identical seed and world
    for (size_t i = 0; i < simulation_count; i++) {
        bool particle_effects = i == 0 && m_hasRealtimeSimulation;
        m_simulations.emplace_back(&m_world, i, m_seed + m_generation, particle_effects, false);
    }
    m_simulationsLeft.store(simulation_count);

    // Partition up the simulations among the threads
    for(int i = 0; i < THREAD_COUNT; i++)
        runSimulationsInThread(oneRealtime ? i+1 : i, THREAD_COUNT, simulation_count);
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

void BaseSimulation::finishGeneration() {
    assert (hasGenerationRunning() && getSimulationsRunning() == 0);

    // All threads should be done by now, but join up for good measure
    for (auto& thread:m_threads)
        thread.join();
    m_threads.clear();

    evolveGenePool();
    m_simulations.clear(); // The scores are stored in the brains, so the simulations are longer needed

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

    // Remove all signs of the generation running, or having ever ran
    m_simulations.clear();
    m_simulationsLeft.store(0);
    m_hasRealtimeSimulation = false;

    m_isGenerationAborted.store(false, std::memory_order_release);
}