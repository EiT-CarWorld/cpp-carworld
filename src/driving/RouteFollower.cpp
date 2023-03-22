#include "RouteFollower.h"
#include "carMath.h"
#include <cmath>

RouteFollower::RouteFollower(const Route *route): m_route(route), m_nextNode{1} {
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

const Node* RouteFollower::getStartNode() {
    return m_route->nodes[0];
}

const Node* RouteFollower::getTarget() {
    return m_target;
}

float RouteFollower::getDistanceToTarget2D(Vector3 position) {
    if (m_target == nullptr)
        return 0;
    Vector3 difference = m_target->position - position;
    difference.y = 0;
    return Vector3Length(difference);
}

float RouteFollower::getTurnInTarget() {
    if (m_nextNode == 0 || m_nextNode == m_route->nodes.size()-1)
        return 0.0f;
    Vector3 prev = m_route->nodes[m_nextNode-1]->position;
    Vector3 target = m_target->position;
    Vector3 future = m_route->nodes[m_nextNode+1]->position;

    Vector2 currentLine{target.x - prev.x, target.y - prev.y};
    Vector2 nextLine{future.x - target.x, future.y - target.y};
    currentLine = Vector2Normalize(currentLine);
    nextLine = Vector2Normalize(nextLine);
    return std::asin(currentLine.x * nextLine.y - nextLine.x * currentLine.y);
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