#include "FloorGrid.h"
#include "raymath.h"
#include "rlgl.h"
#include "carMath.h"

Material FloorGrid::material;
Mesh FloorGrid::planeMesh;
int FloorGrid::gridSizeLoc, FloorGrid::viewPosLoc, FloorGrid::gridColorLoc;

void FloorGrid::loadStatic() {
    material = LoadMaterialDefault();
    material.shader = LoadShader("res/shaders/base.vs", "res/shaders/grid.fs");
    viewPosLoc = GetShaderLocation(material.shader, "viewPos");
    gridSizeLoc = GetShaderLocation(material.shader, "gridSize");
    gridColorLoc = GetShaderLocation(material.shader, "gridColor");

    Image gridImage = GenImageColor(128, 128, BLACK);
    ImageDrawRectangle(&gridImage, 0, 63, 128, 2, WHITE);
    ImageDrawRectangle(&gridImage, 63, 0, 2, 128, WHITE);
    Texture gridTex = LoadTextureFromImage(gridImage);
    UnloadImage(gridImage);

    GenTextureMipmaps(&gridTex);
    SetTextureFilter(gridTex, TEXTURE_FILTER_TRILINEAR);
    SetMaterialTexture(&material, MATERIAL_MAP_DIFFUSE, gridTex);

    planeMesh = GenMeshPlane(1, 1, 1, 1);
}

void FloorGrid::unloadStatic() {
    UnloadMaterial(material);
    UnloadMesh(planeMesh);
}

FloorGrid::FloorGrid(Vector2 size, float gridSize, Color color) :
        m_size(size), m_gridSize(gridSize), m_color(color) {}

void FloorGrid::render(Camera3D camera) {
    SetShaderValue(material.shader, gridSizeLoc, &m_gridSize, SHADER_UNIFORM_FLOAT);
    SetShaderValue(material.shader, viewPosLoc, &camera.position, SHADER_UNIFORM_VEC3);
    Vector3 gridColorAsVec {m_color.r/255.f, m_color.g/255.f, m_color.b/255.f};
    SetShaderValue(material.shader, gridColorLoc, &gridColorAsVec, SHADER_UNIFORM_VEC3);

    Vector2 size = (0.5f + powf(fabs(camera.position.y/5), .5)) * m_size;
    Matrix scaleMatrix = MatrixScale(size.x, 1, size.y);
    Matrix transform = MatrixTranslate(camera.position.x, 0, camera.position.z);
    transform = MatrixMultiply(scaleMatrix, transform);
    rlDisableDepthMask();
    DrawMesh(planeMesh, material, transform);
    rlEnableDepthMask();
}