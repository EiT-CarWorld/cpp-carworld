#include <fstream>
#include <cassert>
#include <algorithm>
#include "Simulation.h"

Simulation::Simulation(World *world, size_t index_in_generation, unsigned long seed, bool store_history)
        : m_world(world), m_index_in_generation(index_in_generation), m_random(seed), m_store_history(store_history), m_frameNumber(0) {}

World* Simulation::getWorld() {
    return m_world;
}

size_t Simulation::getFrameNumber() {
    return m_frameNumber;
}

size_t Simulation::getIndexInGeneration() {
    return m_index_in_generation;
}

void Simulation::spawnCar(size_t route, CarBrain* brain, float spawnRandomness) {
    auto& routes = m_world->getRoutes();
    assert (route < routes.size());
    std::normal_distribution<float> dist(0.f, spawnRandomness);
    m_cars.emplace_back(std::make_unique<Car>(&routes[route], dist(m_random), brain));
}

std::vector<std::unique_ptr<Car>>& Simulation::getCars() {
    return m_cars;
}

// Does all sensing for all cars before performing any updates, will also detect collisions
// This means cars crashing, always take out each other, on the same frame
void Simulation::takeCarActions() {
    // Before asking any cars to pick an action, calculate all car sensors
    for (const auto & m_car : m_cars)
        m_car->calculateSensors(this);

    for(auto& car :m_cars)
        car->chooseAction();
}

void Simulation::updateCars() {
    // Updates all cars, using the actions they last decided on (See: takeCarActions())
    for (int i = 0; i < m_cars.size(); i++) {
        Car* car = m_cars[i].get();
        if (car->hasFinishedRoute() || car->hasCrashed() || car->getScore() <= SCORE_MINIMUM) {
            m_carHasDied = m_carHasDied || car->hasCrashed() || car->getScore() <= SCORE_MINIMUM;

            if (!m_markedAsFinished)
                m_finalCarScores.emplace_back(car->getBrain(), car->getScore());

            std::swap(m_cars[i--], m_cars.back());
            m_cars.pop_back();
            continue;
        }
        car->update();
    }

    if (m_store_history) {
        for (auto& car : m_cars) {
            auto it = m_score_history.find(car.get());
            if (it == m_score_history.end()) {
                m_score_history.insert({car.get(), {m_frameNumber, {car->getScore()}}});
            } else {
                it->second.second.push_back(car->getScore());
            }
        }
    }

    m_frameNumber++;
}

void Simulation::render() {
    m_world->render();
    for(auto& car: m_cars)
        car->render();
}

// Returns true if any car has died
bool Simulation::hasCarDied() {
    return m_carHasDied;
}

// The score of all cars that have finished, or their score at the time finished was marked
std::vector<std::pair<CarBrain*, float>>& Simulation::getFinalCarScores() {
    return m_finalCarScores;
}

// Add up the scores of all finished cars, as well as the ones still alive
float Simulation::getTotalSimulationScore() {
    double sum = 0;
    for (auto& car : m_finalCarScores)
        sum += car.second;

    // If we are not finished, also include cars that are still driving
    if (!m_markedAsFinished) {
        for (auto &car: m_cars)
            sum += car->getScore();
    }
    return (float) sum;
}

// Saves the scores of all cars into the final car scores
void Simulation::markAsFinished() {
    if (m_markedAsFinished)
        return;
    m_markedAsFinished = true;

    for (auto& car : m_cars)
        m_finalCarScores.push_back({car->getBrain(), car->getScore()});
}

bool Simulation::isMarkedAsFinished() {
    return m_markedAsFinished;
}

void Simulation::printHistoryToFile(const std::string& filename) {
    assert(m_store_history);

    std::ofstream out;
    out.open(filename);
    for (auto& data : m_score_history) {
        size_t spawnFrame = data.second.first;
        auto& scores = data.second.second;
        for (int i = 0; i < m_frameNumber; i++) {
            if (i) out << ",";
            if (i < spawnFrame || i-spawnFrame >= scores.size())
                out << 0;
            else
                out << scores[i-spawnFrame];
        }
        out << std::endl;
    }
    out.close();
}