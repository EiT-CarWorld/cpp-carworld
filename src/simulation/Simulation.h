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

    std::mt19937 m_random;

    std::vector<std::unique_ptr<Car>> m_cars{};

    // How many steps of simulation have been performed?
    size_t m_frameNumber;

    // True if any car has been deleted due to crashing
    bool m_carHasDied{false};

    // The scores of all cars, either when they have died, or when marked as finished
    std::vector<std::pair<CarBrain*, float>> m_finalCarScores;

    // Once marked as finished, simulation can continue, but the scores are final
    bool m_markedAsFinished{};

    // Optional functionality for storing a bunch of data from the simulation
    bool m_store_history;
    std::unordered_map<Car*, std::pair<size_t, std::vector<float>>> m_score_history{};
public:
    Simulation(World* world, CarBrain* carBrain, unsigned long seed, bool store_history);

    World* getWorld();
    size_t getFrameNumber();
    void spawnCar(size_t route, CarBrain* brain, float spawnRandomness);
    std::vector<std::unique_ptr<Car>>& getCars();
    void takeCarActions();
    void updateCars();
    void render();
    bool hasCarDied();
    std::vector<std::pair<CarBrain*, float>>& getFinalCarScores();
    float getTotalSimulationScore();
    void markAsFinished();
    bool isMarkedAsFinished();
    void printHistoryToFile(const std::string& filename);
};

