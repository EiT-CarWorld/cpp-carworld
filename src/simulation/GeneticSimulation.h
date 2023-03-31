#pragma once
#include "driving/RandomRoutesPicker.h"
#include "BaseSimulation.h"

// A class for running multiple simulations with different brains, and using their scores to create new brains.
// Multithreading is used internally, but all public functions must be called from the same thread
class GeneticSimulation : public BaseSimulation {
protected:
    virtual void evolveGenePool() override;
public:
    GeneticSimulation();

    virtual void startParallelGeneration(bool oneRealtime) override;

    virtual bool preSimulationFrame(Simulation* simulation) override;
};
