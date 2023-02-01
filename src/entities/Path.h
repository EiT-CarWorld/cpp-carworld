#pragma once
#include "raylib.h"

#define ROAD_WIDTH 8

class Node;

struct PathNode {
    Vector3 position;
};

/*
 * Represents a path between two nodes, possibly going through of a set of path nodes
 * The path can optionally be one-way, in which case it goes from a to b only.
 */
class Path {
public:
    Node *m_a, *m_b;
    bool m_oneway;
    PathNode* m_pathNodes;
    int m_pathNodeCount;

    static Model roadModel, onewayRoadModel, pathNodeModel;
    static Texture asphalt;

    static void loadStatic();
    static void unloadStatic();

    Path(Node* a, Node* b, bool oneway, PathNode* nodes, int nodeCount);
    ~Path() = default;
    void render();
};
