#pragma once
#include <cstddef>
#include "raylib.h"

#define ROAD_WIDTH 8

class Node;
class World;
struct Route;

struct PathNode {
    Vector3 position;
};

/*
 * Represents a path between two nodes, possibly going through of a set of path nodes
 * The path can optionally be one-way, in which case it goes from a to b only.
 */
class Path {
    Node *m_a, *m_b;
    bool m_oneway;
    PathNode* m_pathNodes;
    size_t m_pathNodeCount;

    static Model roadModel, onewayRoadModel, pathNodeModel;
    static Texture asphalt, asphaltSpecular;

    friend Node;
    friend Route;
    friend World;
public:
    static void loadStatic();
    static void unloadStatic();

    Path(Node* a, Node* b, bool oneway, PathNode* nodes, size_t nodeCount);
    Path(const Path& other) = delete;
    Path(Path&& other) = delete;
    ~Path() = default;
    void render();
};
