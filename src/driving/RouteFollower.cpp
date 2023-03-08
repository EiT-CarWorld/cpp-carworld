#include "RouteFollower.h"
#include "carMath.h"

RouteFollower::RouteFollower(Route *route): m_route(route), m_nextNode{1} {
    calculateTarget();
}

void RouteFollower::calculateTarget() {
    if (hasFinishedRoute()) {
        m_target = nullptr;
        return;
    }
    m_nextNode = m_nextNode % m_route->nodes.size();
    m_target = m_route->nodes[m_nextNode];
}

Node* RouteFollower::getStartNode() {
    return m_route->nodes[0];
}

Node* RouteFollower::getTarget() {
    return m_target;
}

float RouteFollower::getDistanceToTarget2D(Vector3 position) {
    if (m_target == nullptr)
        return 0;
    Vector3 difference = m_target->position - position;
    difference.y = 0;
    return Vector3Length(difference);
}

void RouteFollower::updateIfAtTarget(Vector3 position) {
    while (m_target) {
        Vector3 difference = position - m_target->position;
        difference.y = 0;
        if (Vector3Length(difference) > m_target->diameter / 2)
            break;
        m_nextNode++;
        calculateTarget();
    }
}

bool RouteFollower::hasFinishedRoute() {
    return !m_route->loops && m_nextNode >= m_route->nodes.size();
}