#pragma once
#include <cstddef>
#include "raylib.h"
#include "carConfig.h"

class Node;

struct PathNode {
    Vector3 position{};
    float diameter = ROAD_WIDTH;
    void render();
};

/*
 * Represents a path between two nodes, possibly going through of a set of path nodes
 * The path can optionally be one-way, in which case it goes from a to b only.
 */
class Path {
private:
    static Model roadModel, onewayRoadModel, pathNodeModel;
    static Texture asphalt, asphaltSpecular;
    friend Node;
    friend PathNode;
public:
    static void loadStatic();
    static void unloadStatic();

    Node *a, *b;
    bool oneway;
    PathNode* path_nodes;
    size_t path_node_count;

    Path(Node* a, Node* b, bool oneway, PathNode* path_nodes, size_t path_node_count);
    Path(const Path& other) = delete;
    Path(Path&& other) = delete;
    ~Path() = default;
    void render();
};
