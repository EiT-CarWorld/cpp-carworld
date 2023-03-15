#pragma once
#include <string>
#include <vector>
#include <memory>

#include "entities/Node.h"
#include "driving/Route.h"
#include "entities/Car.h"
#include "entities/LineSegment.h"
#include "entities/AsphaltMesh.h"

class World {
    std::vector<Node> m_nodes;

    std::vector<LineSegment> m_lineSegments;

    // A route is a series of edges, possibly looping
    std::vector<Route> m_routes;

    std::unique_ptr<AsphaltMesh> m_asphaltMesh;

    // Once simulations have started, we can no longer change the world,
    // since cars have pointers to our data
public:
    World();

    // These functions should NEVER be called if there are cars currently using the world
    bool loadFromFile(const std::string& path);
    void clearRoutes();
    void addRoute(size_t from, size_t to);

    bool isLoaded();
    const std::vector<Node>& getNodes();
    const std::vector<Route>& getRoutes();
    float getRayDistance(Vector2 pos, Vector2 dir, float max_distance);
    void render();
    void renderRoadBorders();
};
