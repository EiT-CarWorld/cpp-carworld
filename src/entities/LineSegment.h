#pragma once
#include "raylib.h"
#include "carMath.h"
#include <optional>

struct LineSegment {
    // Line segments are defined using Vector2 in (x=east,y=north) space
    Vector2 a, b;
    // Gets the distance from a ray. The ray is cast in (x=east,y=north) space
    std::optional<float> getRayDistance(Vector2 pos, Vector2 dir);
    // Renders the line in (x=east,z=south) space
    void render();
};
