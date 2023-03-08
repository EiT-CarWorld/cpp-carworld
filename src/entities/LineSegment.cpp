#include "LineSegment.h"

std::optional<float> LineSegment::getRayDistance(Vector2 pos, Vector2 dir) {
    float x1 = a.x, y1 = a.y;
    float x2 = b.x, y2 = b.y;

    float x3 = pos.x, y3 = pos.y;
    float x4 = pos.x+dir.x, y4 = pos.y+dir.y;

    // Taken from http://www.jeffreythompson.org/collision-detection/line-line.php
    float uA = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
    if (uA < 0 || uA > 1)
        return std::nullopt;
    float uB = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
    if (uB < 0)
        return std::nullopt;
    return uB * Vector2Length(dir);
}

void LineSegment::render() {
    DrawLine3D({a.x, 0.5, -a.y}, {b.x, 0.5, -b.y}, BLUE);
}
