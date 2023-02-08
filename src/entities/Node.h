#pragma once
#include <vector>
#include "Path.h"

class Node : public PathNode {
public:
    std::vector<Path*> paths;
    explicit Node(Vector3 position) : PathNode{position, ROAD_WIDTH+2}, paths({}) {}
    ~Node() = default;
    void addedToPath(Path *path);
};
