#pragma once
#include <vector>
#include <memory>
#include <random>
#include "entities/World.h"
#include "entities/Car.h"

// Represents one simulation, on a given map
class Simulation {
    World* m_world;
    std::vector<std::unique_ptr<Car>> m_cars;

    std::mt19937 m_random;
public:
    Simulation(World* world, unsigned long seed);

    World* getWorld();

    void spawnCar();
    std::vector<std::unique_ptr<Car>>& getCars();
    void takeCarActions();
    void updateCars();

    void render();
};

