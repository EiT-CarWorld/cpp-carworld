#include "CarZonesVisualizer.h"
#include "raymath.h"

const float ZONE_MESH_SIZE = 100.f;

Mesh CarZonesVisualizer::carZoneMesh;
Material CarZonesVisualizer::material;
int CarZonesVisualizer::carPositionLoc;
int CarZonesVisualizer::carYawLoc;
int CarZonesVisualizer::zoneLengthsLoc;

void CarZonesVisualizer::loadStatic() {
    carZoneMesh = GenMeshPlane(ZONE_MESH_SIZE, ZONE_MESH_SIZE, 1, 1);

    material = LoadMaterialDefault();
    material.shader = LoadShader("res/shaders/base.vs", "res/shaders/carZones.fs");
    carPositionLoc = GetShaderLocation(material.shader, "carPosition");
    carYawLoc = GetShaderLocation(material.shader, "carYaw");
    zoneLengthsLoc = GetShaderLocation(material.shader, "zoneLengths");
}

void CarZonesVisualizer::unloadStatic() {
    UnloadMaterial(material);
    UnloadMesh(carZoneMesh);
}

void CarZonesVisualizer::DrawCarZones(Vector3 position, float carYaw, float zoneDistances[]) {
    Vector2 carPosition{position.x, position.z};
    SetShaderValue(material.shader, carPositionLoc, &carPosition, SHADER_UNIFORM_VEC2);
    SetShaderValue(material.shader, carYawLoc, &carYaw, SHADER_UNIFORM_FLOAT);
    SetShaderValueV(material.shader, zoneLengthsLoc, zoneDistances, SHADER_UNIFORM_FLOAT, NUM_CAR_ZONES);
    DrawMesh(carZoneMesh, material, MatrixTranslate(position.x, position.y, position.z));
}