#pragma once
#include <vector>
#include <atomic>
#include <map>
#include <thread>
#include <fstream>
#include "Simulation.h"

// A class for running multiple simulations with different brains, and using their scores to create new brains.
// Multithreading is used internally, but all public functions must be called from the same thread
class GeneticSimulation {
    World m_world{};
    // The seed used for every simulation
    unsigned long m_seed{};
    // For each frame number, contains what route should have a car spawned
    std::multimap<size_t, size_t> m_carSpawnTimes{};

    // How many brains to simulate per generations
    size_t m_poolSize{};
    // How many brains are kept to the next generation
    size_t m_survivorsPerGeneration{};
    // How long each simulation runs for before its score is evaluated
    size_t m_framesPerSimulation{};
    // How likely any value in a matrix is to be replaced
    float m_mutationChance{};
    // To avoid overfitting, let cars spawn slightly different locations each generation
    float m_spawnRandomness{};

    // How many generations have been simulated / are being simulated
    size_t m_generation{0};
    // The brains being simulated this generation, or the survivors from previous
    std::vector<CarBrain> m_geneticPool;
    // The file output for printing brain scores every round
    std::ofstream m_brainScoreOutput{};

    // The rest of the member variables only apply during a generation
    // To allow aborting of generations, we remember how many parents to keep in the gene pool
    size_t m_parentsThisGeneration{};
    // All simulations of this generation. A 1:1-mapping between this list and the geneticPool
    std::vector<Simulation> m_simulations{};
    // If true, simulation number 0 is not executed by the threads
    bool m_hasRealtimeSimulation{};
    // The number of simulations in the generation that still have frames left
    std::atomic<size_t> m_simulationsLeft{};
    // All threads running simulations in the background
    std::vector<std::thread> m_threads{};
    // To allow the eventual shutoff of the program
    std::atomic<bool> m_isGenerationAborted{false};

    // Uses the existing brains in the pool, to create new ones
    void fillGenePool();
    // Keeps only the best brains, run after a complete generation
    void pruneGenePool();
    // Spawns a thread to run the simulations in the half-open interval [begin, end)
    void runSimulationsInThread(size_t begin, size_t end);
public:
    GeneticSimulation(std::vector<CarBrain> initial_brains);

    size_t getGenerationNumber();
    size_t getFramesPerSimulation();

    bool loadParameterFile(const char* path);
    bool loadGenePool(const char *path);
    bool saveGenePool(const char* path);

    void setScoreOutputFile(const char* path);

    // Returns true if the genetic simulation is in the middle of a generation.
    // Becomes true with startParallelGeneration()
    // Becomes false with finishGeneration() or abortGeneration()
    bool hasGenerationRunning();
    // Starts an entire generation worth of simulations
    // They run in one or more threads.
    // If oneRealtime is true, simulation #0 is are not executed by any of the threads.
    // Instead, it is left up to the driver to update it until enough frames have been simulated.
    // If all methods on this class are called from the same thread, everything is safe
    void startParallelGeneration(bool oneRealtime);
    // Returns a simulation if the currently running generation has a realtime simulation
    Simulation* getRealtimeSimulation();
    // Called on each simulation frame before updating, to e.g. spawn cars,
    // If the simulation has been run for enough frames, it will save the score, and start returning false
    bool preSimulationFrame(Simulation* simulation);
    // How many simulations are running currently.
    // Will be 0 if no generation is currently running
    // Background simulations stop once they reach the frame count.
    // A potential realtime simulation can still run after enough frames, but the score is frozen
    size_t getSimulationsRunning();
    // Once no more simulations are running, we can finish the generation
    // This prepares the brains for the next generation
    void finishGeneration();
    // Shuts down all threads, and aborts the generation without learning anything
    // Also decreases the generation number, to make it clear
    void abortGeneration();
};
