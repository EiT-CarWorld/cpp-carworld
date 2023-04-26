#pragma once
#include <vector>
#include <memory>
#include <random>
#include <unordered_map>
#include "entities/World.h"
#include "entities/Car.h"
#include "rendering/ParticleEffect.h"

// Represents one simulation, on a given map
class Simulation {
    World* m_world;
    // Which simulation within the generation we belong to
    size_t m_index_in_generation;
    // Deterministic source of randomness for this Simulation
    std::mt19937 m_random;

    // How many steps of simulation have been performed?
    size_t m_frameNumber{0};

    std::vector<std::unique_ptr<Car>> m_cars{};

    // How many cars have been spawned in this world in total
    size_t m_num_spawned_cars{};
    // How many cars have crashed (either into each other or into the ditch)
    size_t m_num_dead_cars{};

    // Used to calculate the average amount of cars
    size_t m_integral_of_cars_per_frame{};

    // True if any car has been deleted due to crashing
    bool m_carHasDied{false};

    // The scores of all cars, either when they have died, or when marked as finished
    std::vector<std::pair<CarBrain*, float>> m_finalCarScores;

    // Once marked as finished, simulation can continue, but the scores are final
    bool m_markedAsFinished{};

    // Optional functionality for storing a bunch of data from the simulation
    bool m_store_history;
    std::unordered_map<Car*, std::pair<size_t, std::vector<float>>> m_score_history{};

    bool m_draw_particle_effects;
    std::vector<ParticleEffect> m_particleEffects;
public:
    Simulation(World* world, size_t index_in_generation, size_t seed, bool draw_particle_effects, bool store_history);

    World* getWorld();
    size_t getFrameNumber();
    size_t getIndexInGeneration();
    size_t getNumberOfSpawnedCars();
    size_t getNumberOfDeadCars();

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

