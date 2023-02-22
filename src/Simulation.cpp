#include "Simulation.h"

Simulation::Simulation(World *world, unsigned long seed) : m_world{world}, m_random(seed) {}

World* Simulation::getWorld() {
    return m_world;
}

void Simulation::spawnCar() {
    auto& routes = m_world->getRoutes();
    std::uniform_int_distribution<size_t> route_choice(0, routes.size()-1);
    m_cars.emplace_back(std::make_unique<Car>(&routes[route_choice(m_random)]));
}

std::vector<std::unique_ptr<Car>>& Simulation::getCars() {
    return m_cars;
}

void Simulation::takeCarActions() {
    // Before asking any cars to pick an action, calculate all car sensors
    for (const auto & m_car : m_cars)
        m_car->calculateSensors(this);

    for(auto& car:m_cars)
        car->chooseAction();
}

void Simulation::updateCars() {
    // Updates all cars, using the actions they last decided on (See: takeCarActions())
    for (const auto & m_car : m_cars)
        m_car->update();
}

void Simulation::render() {
    m_world->render();
    for(auto& car: m_cars)
        car->render();
}