#pragma once
#include <vector>
#include "raylib.h"
#include "carConfig.h"

class Node {
public:
    Vector3 position{};
    float diameter = ROAD_WIDTH;
    // We only include nodes we can drive to from this node
    std::vector<Node*> neighbours;
    explicit Node(Vector3 position) : position(position), neighbours() {}
    ~Node() = default;
    void addNeighbour(Node* node);
    void renderCircle(Color color) const;
};
