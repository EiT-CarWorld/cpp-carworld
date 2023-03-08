#include "Node.h"

void Node::addedToEdge(Edge *edge) {
    edges.push_back(edge);
}

void Node::render() {
    DrawModelEx(Edge::nodeModel, position,
                {0,1,0}, 0, {diameter, 1, diameter}, WHITE);
}