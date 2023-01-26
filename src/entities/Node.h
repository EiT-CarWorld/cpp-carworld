#pragma once
#include "raylib.h"
#include "Path.h"

class Node {
private:
    static Model roadModel, pathNodeModel;
    static Texture asphalt;

    Path* m_outpaths[2];
    Path* m_inpaths[2];
public:
    static void loadStatic();
    static void unloadStatic();

    Vector3 m_position;
    Node(Vector3 position) : m_position(position), m_inpaths(), m_outpaths() {}
    ~Node() = default;
    void addOutPath(Path *path);
    void addInPath(Path *path);
    void render();
};
