#include "Node.h"

void Node::addNeighbour(Node *n) {
    neighbours.push_back(n);
}

void Node::renderCircle(Color color) const {
    Vector3 pos = position;
    pos.y += 2;
    DrawCircle3D(pos, ROAD_WIDTH/2, {1,0,0}, 90.f, color);
}