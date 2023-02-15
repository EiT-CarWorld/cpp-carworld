#pragma once
#include "raylib.h"
#include "carMath.h"
#include <optional>

struct LineSegment {
    Vector2 a, b;
    std::optional<float> getRayDistance(Vector2 pos, Vector2 dir);
    void render();
};
