#pragma once
#include "raylib.h"

class AsphaltMesh {
    static Texture asphaltTexture, specularTexture;
public:
    static void loadStatic();
    static void unloadStatic();

private:
    int num_vertices;
    int num_indices;
    unsigned int vaoId;
    unsigned int positionVboId, indicesVboId;
public:
    AsphaltMesh(float* vertices, int num_vertices, unsigned int* indices, int num_indices);
    ~AsphaltMesh();
    void render();
};

