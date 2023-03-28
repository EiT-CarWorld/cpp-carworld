#pragma once
#include "driving/RandomRoutesPicker.h"
#include "BaseSimulation.h"

// A class for running multiple simulations with different brains, and using their scores to create new brains.
// Multithreading is used internally, but all public functions must be called from the same thread
class GeneticSimulation : public BaseSimulation {
    // For each frame number, contains what route should have a car spawned
    std::multimap<size_t, size_t> m_carSpawnTimes{};

    // To get added learning, the set of car spawn times can be randomized every Nth frame
    RandomRoutesPicker m_routesPicker;

protected:
    // Handle options passed in from a config file
    virtual bool handleOption(std::string& opt, std::ifstream& file, bool ignoreSaveLoad) override;
public:
    explicit GeneticSimulation(std::vector<CarBrain> initial_brains);

    virtual void startParallelGeneration(bool oneRealtime) override;

    virtual bool preSimulationFrame(Simulation* simulation) override;
};
