#pragma once
#include <vector>
#include <atomic>
#include <map>
#include <thread>
#include <fstream>
#include "Simulation.h"
#include "driving/RandomRoutesPicker.h"

class BaseSimulation {
protected:
    World m_world{};
    // The seed used for every simulation
    unsigned long m_seed{};

    // For each frame number, contains what route should have a car spawned
    std::multimap<size_t, size_t> m_carSpawnTimes{};
    // To get added learning, the set of car spawn times can be randomized every Nth frame
    RandomRoutesPicker m_routesPicker;

    // The size of each hidden layer in the brain
    std::vector<size_t> m_brain_layers{};
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

    // All simulations of this generation
    std::vector<Simulation> m_simulations{};
    // If true, simulation number 0 is not executed by the threads
    bool m_hasRealtimeSimulation{};
    // If true, only one simulation is running, the realtime one
    bool m_singleSimulationRunning{};
    // The number of simulations in the generation that still have frames left
    std::atomic<size_t> m_simulationsLeft{};
    // All threads running simulations in the background
    std::vector<std::thread> m_threads{};
    // To allow the eventual shutoff of the program
    std::atomic<bool> m_isGenerationAborted{false};

    // Adds or removes brains until gene pool is the correct size
    // Uses m_seed and m_brain_layers to make new brains, if needed
    void fitGenePoolToSize();

    void startParallelGeneration(bool oneRealtime, size_t simulation_count);

    // Spawns a thread for each offset + (i*stride) up to, but not including end
    void runSimulationsInThread(size_t offset, size_t stride, size_t end);

    // Gets called at the end of a generation, before the simulations are cleared.
    virtual void evolveGenePool()=0;
    // Uses the set of (score, brain), to evolve the brains. Should be sorted highest first.
    void geneticEvolveGenePool(std::vector<std::pair<float, int>> const& scores);
    // Prints out each score, the scores should have been sorted
    void printBrainScores(std::vector<std::pair<float, int>> const& scores);

    virtual bool handleOption(std::string& opt, std::ifstream& file, bool ignore_gene_pool);
public:
    explicit BaseSimulation();

    size_t getGenerationNumber();
    size_t getFramesPerSimulation();
    CarBrain* getBestBrain();

    bool loadParameterFile(const char* path, bool ignoreSaveLoad);
    bool loadGenePool(const char *path);
    bool saveGenePool(const char* path);
    bool loadParameterFileIfExists(const char* path, bool ignoreSaveLoad);

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
    virtual void startParallelGeneration(bool oneRealtime)=0;

    // Uses the first brain in the gene pool to show a realtime simulation, without doing any learning
    void startSingleSimulation();

    // Returns a simulation if the currently running generation has a realtime simulation
    Simulation* getRealtimeSimulation();

    // Called on each simulation frame before updating, to e.g. spawn cars,
    // If the simulation has been run for enough frames, it will save the score, and start returning false
    virtual bool preSimulationFrame(Simulation* simulation) = 0;

    // How many simulations are running currently.
    // Will be 0 if no generation is currently running
    // Background simulations stop once they reach the frame count.
    // A potential realtime simulation can still run after enough frames, but the score is frozen
    size_t getSimulationsRunning();
    // Once no more simulations are running, we can finish the generation
    // This uses the results to evolve brains for the next generation
    void finishGeneration();
    // Shuts down all threads, and aborts the generation without learning anything
    void abortGeneration();
};

#define OR_RETURN(action) if(!(action)) return false
#define OR_COMPLAIN(conditional) do if(!(conditional)) {             \
std::cerr << "error: " #conditional << std::endl; \
return false;                                                        \
} while(false)
