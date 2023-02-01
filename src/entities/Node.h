#pragma once
#include <vector>
#include "Path.h"

class Node {
public:
    Vector3 m_position;
    std::vector<Path*> m_paths;
    explicit Node(Vector3 position) : m_position(position), m_paths({}) {}
    ~Node() = default;
    void addedToPath(Path *path);
    void render();
};
