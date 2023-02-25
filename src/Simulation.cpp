#include <fstream>
#include <cassert>
#include "Simulation.h"

Simulation::Simulation(World *world, CarBrain* carBrain, unsigned long seed, bool store_history)
        : m_world(world), m_carBrain(carBrain), m_random(seed), m_store_history(store_history), m_frameNumber(0) {}

World* Simulation::getWorld() {
    return m_world;
}

size_t Simulation::getFrameNumber() {
    return m_frameNumber;
}

void Simulation::spawnCar() {
    auto& routes = m_world->getRoutes();
    std::uniform_int_distribution<size_t> route_choice(0, routes.size()-1);
    m_cars.emplace_back(std::make_unique<Car>(&routes[route_choice(m_random)], m_carBrain));
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
    for (auto& car : m_cars)
        car->update();

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

float Simulation::getTotalSimulationScore() {
    double sum = 0;
    for(auto& car:m_cars)
        sum += car->getScore();
    return (float) sum;
}

void Simulation::storeTotalScoreInBrain() {
    m_carBrain->setEvaluationScore(getTotalSimulationScore());
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