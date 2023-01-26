#pragma once
#define ROAD_WIDTH 3

class Node;

struct PathNode {
    Vector3 position;
};

/*
 * Represents a path between two nodes, possibly going through of a set of path nodes
 */
struct Path {
    Node *from, *to;
    PathNode* pathNodes;
    int pathNodeCount;
    Path(PathNode* nodes, int nodeCount) : from(nullptr), to(nullptr), pathNodes(nodes), pathNodeCount(nodeCount) {}
};
