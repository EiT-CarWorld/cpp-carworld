#include "RouteFollower.h"
#include "carMath.h"

RouteFollower::RouteFollower(Route *route): m_route(route), m_pathIndex{0}, m_pathNodeIndex{0} {
    calculateTarget();
}

void RouteFollower::calculateTarget() {
    if (hasFinishedRoute()) {
        m_target = nullptr;
        return;
    }

    Path* currentPath = m_route->paths[m_pathIndex];
    if (m_pathNodeIndex == currentPath->path_node_count) {
        m_target = m_route->nodes[m_pathIndex + 1];
    }
    else {
        Node* nextNode = m_route->nodes[m_pathIndex + 1];
        if (nextNode == currentPath->b) { // The currentPath goes from a to b
            m_target = &currentPath->path_nodes[m_pathNodeIndex];
        } else { // The currentPath goes from b to a, aka the path nodes go in reverse
            m_target = &currentPath->path_nodes[currentPath->path_node_count - 1 - m_pathNodeIndex];
        }
    }
}

Node* RouteFollower::getStartNode() {
    return m_route->nodes[0];
}

PathNode* RouteFollower::getTarget() {
    return m_target;
}

void RouteFollower::updateIfAtTarget(Vector3 position) {
    while (m_target) {
        float distance = Vector3Length(m_target->position - position);
        if (distance >= m_target->diameter / 2)
            break; // We haven't reached the target yet

        // We have reached the target, and need a new one.
        // Increase target index either along the path we are on,
        // or move to the next path
        if (m_pathNodeIndex < m_route->paths[m_pathIndex]->path_node_count)
            // We have more of the current Path to travel
            m_pathNodeIndex++;
        else {
            // We reached the Node at the end of the current path
            m_pathNodeIndex = 0;
            m_pathIndex++;
            //
            if (m_pathIndex >= m_route->paths.size() && m_route->loops)
                m_pathIndex = 0;
        }
        calculateTarget();
    }
}

bool RouteFollower::hasFinishedRoute() {
    return m_pathIndex >= m_route->paths.size();
}