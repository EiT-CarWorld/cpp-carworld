#pragma once
#include <cstddef>
#include "driving/Route.h"

class RouteFollower {
private:
    // The route we are following, from node 0 and up
    Route* m_route;

    // The index of the next target node
    size_t m_nextNode;

    // The current target, updated whenever nextNode is updated
    Node* m_target{nullptr};

    // Calculates the target, and wraps nextNode to 0 if looping
    void calculateTarget();
public:
    explicit RouteFollower(Route* route);

    Node* getStartNode();
    Node* getTarget();
    float getDistanceToTarget2D(Vector3 position);

    // Changes the target, if the car has reached it
    // Also awards the car score for reaching goals, depending on the distance to the last goal
    void updateIfAtTarget(Vector3 position);
    bool hasFinishedRoute();
};
