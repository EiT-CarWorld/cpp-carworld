#pragma once

#include "BaseSimulation.h"

class AdversarialSimulation : public BaseSimulation {
protected:
    virtual void evolveGenePool() override;
public:
    explicit AdversarialSimulation();

    virtual void startParallelGeneration(bool oneRealtime) override;

    virtual bool preSimulationFrame(Simulation* simulation) override;
};
