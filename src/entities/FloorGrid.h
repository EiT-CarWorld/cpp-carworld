#pragma once
#include "raylib.h"

class FloorGrid {
private:
    static Material material;
    static int gridSizeLoc, viewPosLoc, gridColorLoc;
    static Mesh planeMesh;

    Vector2 m_size;
    float m_gridSize;
    Color m_color;
public:
    static void loadStatic();
    static void unloadStatic();

    FloorGrid(Vector2 size, float gridSize, Color color);
    void render(Camera3D camera);
};

