#pragma once
#include <string>
#include <vector>
#include <memory>

#include "entities/Node.h"
#include "entities/Path.h"
#include "driving/Route.h"
#include "entities/Car.h"
#include "entities/LineSegment.h"

class World {
    std::vector<Node> m_nodes;
    std::vector<PathNode> m_pathNodes;

    // A path connects two Nodes, using a continuous segment of the path_nodes list
    std::vector<std::unique_ptr<Path>> m_paths;

    std::vector<LineSegment> m_lineSegments;

    // A route is a series of paths, possibly looping
    std::vector<Route> m_routes;

    // The cars of our world
    std::vector<std::unique_ptr<Car>> m_cars;

public:
    World();

    void loadFromFile(const std::string& path);
    void createRoutes(unsigned seed, size_t count);
    float getRayDistance(Vector2 pos, Vector2 dir, float max_distance);
    void spawnCar();
    std::vector<std::unique_ptr<Car>>& getCars();
    void takeCarActions();
    void updateCars();
    void render();
    void renderRoadBorders();
};
