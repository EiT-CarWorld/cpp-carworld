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

    // True if any car has been deleted due to crashing
    bool m_carHasDied{false};

    // The sum of scores of cars that have been removed
    float m_finishedCarsScore{};

    // Once marked as finished, simulation can continue, but the score is final
    bool m_markedAsFinished{};

    // Optional functionality for storing a bunch of data from the simulation
    bool m_store_history;
    std::unordered_map<Car*, std::pair<size_t, std::vector<float>>> m_score_history{};

    std::mt19937 m_random;
public:
    Simulation(World* world, CarBrain* carBrain, unsigned long seed, bool store_history);

    World* getWorld();
    size_t getFrameNumber();
    void spawnCar(size_t route, float spawnRandomness);
    std::vector<std::unique_ptr<Car>>& getCars();
    void takeCarActions();
    void updateCars();
    void render();
    bool hasCarDied();
    float getTotalSimulationScore();
    void markAsFinished();
    bool isMarkedAsFinished();
    void printHistoryToFile(const std::string& filename);
};

