#pragma once
#include <vector>
#include "Simulation.h"

// A class for running multiple simulations with different brains, and using their scores to create new brains
class GeneticSimulation {
    World* m_world;
    unsigned long m_seed;
    std::vector<int> m_carSpawnTimes;

    size_t m_poolSize;
    size_t m_survivorsPerGeneration;
    size_t m_framesPerSimulation;

    // How many generations have been simulated
    size_t m_generation;

    std::vector<CarBrain> m_geneticPool;

    std::vector<Simulation> m_simulations;
    bool m_hasRealtimeSimulation;

public:
    GeneticSimulation(World* world, unsigned long m_seed, std::vector<int> carSpawnTimes,
                      size_t m_poolSize, size_t m_survivorsPerGeneration, size_t m_framesPerSimulation);

    // Starts an entire generation worth of simulations
    // They run in one or more threads.
    // If oneRealtime is true, simulation #0 is are not executed by any of the threads.
    // Instead, it is left up to the driver to update it until enough frames have been simulated.
    // If all methods on this class are called from the same thread, everything is safe
    void startParallelGeneration(bool oneRealtime);
    // If some simulations in the generation are still running, their count is returned
    // Otherwise, the result of the simulations
    int finishGeneration();
    // Returns a simulation if the currently running generation has a realtime simulation
    Simulation* getRealtimeSimulation();
};
