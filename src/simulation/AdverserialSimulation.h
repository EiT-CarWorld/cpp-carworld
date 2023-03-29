#pragma once

#include "BaseSimulation.h"

class AdverserialSimulation : public BaseSimulation {
protected:
    // Handle options passed in from a config file
    virtual bool handleOption(std::string& opt, std::ifstream& file, bool ignoreSaveLoad) override;
public:
    explicit AdverserialSimulation(std::vector<CarBrain> initial_brains);

    virtual void startParallelGeneration(bool oneRealtime) override;

    virtual bool preSimulationFrame(Simulation* simulation) override;
};
