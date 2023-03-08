#pragma once
#include <vector>
#include "Edge.h"

class Node {
public:
    Vector3 position{};
    float diameter = ROAD_WIDTH;
    std::vector<Edge*> edges;
    explicit Node(Vector3 position) : position(position), edges({}) {}
    ~Node() = default;
    void addedToEdge(Edge *edge);
    void render();
};
