#include <fstream>
#include <cassert>
#include <algorithm>
#include "Simulation.h"

Simulation::Simulation(World *world, CarBrain* carBrain, unsigned long seed, bool store_history)
        : m_world(world), m_carBrain(carBrain),
        m_random(seed), m_store_history(store_history), m_frameNumber(0) {}

World* Simulation::getWorld() {
    return m_world;
}

size_t Simulation::getFrameNumber() {
    return m_frameNumber;
}

void Simulation::spawnCar(size_t route, float spawnRandomness) {
    auto& routes = m_world->getRoutes();
    assert (route < routes.size());
    std::normal_distribution<float> dist(0.f, spawnRandomness);
    m_cars.emplace_back(std::make_unique<Car>(&routes[route], dist(m_random), m_carBrain));
}

std::vector<std::unique_ptr<Car>>& Simulation::getCars() {
    return m_cars;
}

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
        if (m_cars[i]->hasFinishedRoute() || m_cars[i]->hasCrashed() || m_cars[i]->getScore() <= SCORE_MINIMUM) {
            m_carHasDied = m_carHasDied || m_cars[i]->hasCrashed() || m_cars[i]->getScore() <= SCORE_MINIMUM;

            m_finishedCarsScore += m_cars[i]->getScore();

            std::swap(m_cars[i--], m_cars.back());
            m_cars.pop_back();
            continue;
        }
        m_cars[i]->update();
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

bool Simulation::hasCarDied() {
    return m_carHasDied;
}

// Add up the scores of all finished cars, as well as the ones still alive
float Simulation::getTotalSimulationScore() {
    double sum = m_finishedCarsScore;
    for(auto& car:m_cars)
        if (car->getBrain() == m_carBrain)
            sum += car->getScore();
    return (float) sum;
}

void Simulation::markAsFinished() {
    m_carBrain->setEvaluationScore(getTotalSimulationScore());
    m_markedAsFinished = true;
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