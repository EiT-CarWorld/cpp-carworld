#pragma once
#include <cstddef>
#include "driving/Route.h"

class RouteFollower {
private:
    // The route we are following, from node 0 and up
    Route* m_route;

    // Which path along the route we are on.
    size_t m_route_path_index;

    // Which PathNode along the given path we are on
    // -1 means we target the path's start node
    // 0 means we target the 0th PathNode
    // if this variable is equal to path_node_count, the Node ending the path is the target
    // Remember that paths can be traversed both from a->b and b->a
    size_t m_route_path_pathnode_index;

    // The current target, either a PathNode or a Node, based on the variables defined above
    PathNode* m_target{};

    // Performs the logic from above to calculate the target
    void calculateTarget();

public:
    explicit RouteFollower(Route* route);

    Node* getStartNode();
    PathNode* getTarget();

    // Changes the target, if the car has reached it
    void updateIfAtTarget(Vector3 position);
    bool hasFinishedRoute();
};
