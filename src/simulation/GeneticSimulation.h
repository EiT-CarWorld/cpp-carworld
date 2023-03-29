#pragma once
#include "driving/RandomRoutesPicker.h"
#include "BaseSimulation.h"

// A class for running multiple simulations with different brains, and using their scores to create new brains.
// Multithreading is used internally, but all public functions must be called from the same thread
class GeneticSimulation : public BaseSimulation {
protected:
    // Handle options passed in from a config file
    virtual bool handleOption(std::string& opt, std::ifstream& file, bool ignoreSaveLoad) override;
public:
    explicit GeneticSimulation(std::vector<CarBrain> initial_brains);

    virtual void startParallelGeneration(bool oneRealtime) override;

    virtual bool preSimulationFrame(Simulation* simulation) override;
};
