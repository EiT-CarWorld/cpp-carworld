#pragma once

#include "raylib.h"
#include "entities/Car.h"

class CarZonesVisualizer {
private:
    static Material material;
    static Mesh carZoneMesh;
    static int carPositionLoc;
    static int carYawLoc;
    static int zoneLengthsLoc;
public:
    static void loadStatic();
    static void unloadStatic();
    static void DrawCarZones(Vector3 position, float carYaw, float zoneDistances[]);
};

