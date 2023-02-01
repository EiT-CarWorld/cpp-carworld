#pragma once
#include <string>
#include <vector>

#include "Node.h"
#include "Path.h"

class World {
    std::vector<Node> m_nodes;
    std::vector<PathNode> m_pathNodes;
    std::vector<Path> m_paths;

public:
    World();

    void loadFromFile(const std::string& path);
    void render();
};
