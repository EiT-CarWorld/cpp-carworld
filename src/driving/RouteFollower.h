#pragma once
#include <cstddef>
#include "driving/Route.h"

class RouteFollower {
private:
    // The route we are following, from node 0 and up
    const Route* m_route;

    // The index of the next target node
    size_t m_nextNode;

    // The current target, updated whenever nextNode is updated
    const Node* m_target{nullptr};

    // Calculates the target, and wraps nextNode to 0 if looping
    void calculateTarget();
public:
    explicit RouteFollower(const Route* route);

    const Node* getStartNode();
    const Node* getTarget();
    float getDistanceToTarget2D(Vector3 position);
    // Gets the angle that must be turned in the next target, 0 is straight ahead
    float getTurnInTarget();

    // Changes the target, if the car has reached it
    // Also awards the car score for reaching goals, depending on the distance to the last goal
    void updateIfAtTarget(Vector3 position);
    bool hasFinishedRoute();
};
