#pragma once
#include <cstddef>
#include "raylib.h"
#include "carConfig.h"

class Node;

/*
 * Represents a path between two nodes, possibly going through of a set of path nodes
 * The path can optionally be one-way, in which case it goes from a to b only.
 */
class Edge {
private:
    static Model roadModel, onewayRoadModel, nodeModel;
    static Texture asphalt, asphaltSpecular;
    friend Node;
public:
    static void loadStatic();
    static void unloadStatic();

    Node *a, *b;
    bool oneway;

    Edge(Node* a, Node* b, bool oneway);
    ~Edge() = default;
    // Call this once the edge has a fixed address
    void attach();
    void render();
};
