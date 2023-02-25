#pragma once
#include <vector>
#include <atomic>
#include <map>
#include <thread>
#include "Simulation.h"

// A class for running multiple simulations with different brains, and using their scores to create new brains
class GeneticSimulation {
    World* m_world;
    unsigned long m_seed;
    // For each frame number, contains how many cars should be spawned on that frame
    std::unordered_map<size_t, size_t> m_carSpawnTimes;

    // How many brains to simulate per generations
    size_t m_poolSize;
    // How many brains are kept to the next generation
    size_t m_survivorsPerGeneration;
    // How long each simulation runs for before its score is evaluated
    size_t m_framesPerSimulation;

    // How many generations have been simulated / are being simulated
    size_t m_generation;
    std::vector<CarBrain> m_geneticPool;

    // The number of simulations in the generation that still have frames left
    std::atomic<size_t> m_simulationsLeft{};
    // If true, simulation number 0 is not executed by the threads
    bool m_hasRealtimeSimulation{};
    // All simulations of this generation. A 1:1-mapping between this list and the geneticPool
    std::vector<Simulation> m_simulations{};

    // All threads running simulations in the background
    std::vector<std::thread> m_threads{};

    // To allow the eventual shutoff of the program
    bool m_isAborted{false};

    // Uses the existing brains in the pool, to create new ones
    void fillGenePool();
    // Spawns a thread to run the simulations in the half-open interval [begin, end)
    void runSimulationsInThread(size_t begin, size_t end);
public:
    GeneticSimulation(World* world, unsigned long seed, std::unordered_map<size_t, size_t> carSpawnTimes,
                      std::vector<CarBrain> initial_brains,
                      size_t poolSize, size_t survivorsPerGeneration, size_t framesPerSimulation);

    size_t getGenerationNumber();
    size_t getFramesPerSimulation();

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
    // Background simulations stop once they reach the frame count.
    // A potential realtime simulation can still run after enough frames, but the score is frozen
    size_t getCurrentlyRunning();
    // Once no more simulations are running, we can finish the generation
    // This prepares the brains for the next generation
    void finishGeneration();
    // Use this to shut down all threads gracefully
    void abortGeneration();
};
