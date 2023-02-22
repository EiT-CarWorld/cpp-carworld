#pragma once
#include <vector>
#include <memory>
#include <random>
#include <unordered_map>
#include "entities/World.h"
#include "entities/Car.h"

// Represents one simulation, on a given map
class Simulation {
    World* m_world;
    CarBrain* m_carBrain;
    std::vector<std::unique_ptr<Car>> m_cars{};

    // How many steps of simulation have been performed?
    size_t m_frameNumber;

    bool m_store_history;
    std::unordered_map<Car*, std::pair<size_t, std::vector<float>>> m_score_history{};

    std::mt19937 m_random;
public:
    Simulation(World* world, CarBrain* carBrain, unsigned long seed, bool store_history);

    World* getWorld();
    size_t getFrameNumber();
    void spawnCar();
    std::vector<std::unique_ptr<Car>>& getCars();
    void takeCarActions();
    void updateCars();
    void render();
    float getTotalSimulationScore();
    void printHistoryToFile(const std::string& filename);
};

