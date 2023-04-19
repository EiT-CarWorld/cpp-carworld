#pragma once

#include "BaseSimulation.h"

class AverageScoreSimulations : public BaseSimulation {
protected:
    virtual void evolveGenePool() override;
public:
    explicit AverageScoreSimulations();

    virtual void startParallelGeneration(bool oneRealtime) override;

    virtual bool preSimulationFrame(Simulation* simulation) override;
};
