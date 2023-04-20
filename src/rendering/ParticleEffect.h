#pragma once

#include <cstddef>
#include <raylib.h>

class ParticleEffect {
    Vector3 m_position;
    Color m_color;

    size_t m_framesLived{0};
    size_t m_maxAge;
    float m_maxRadius;

    public:
    ParticleEffect(Vector3 position, Color color, size_t maxAge, float maxRadius);

    // Returns false if the particle effect should be removed
    bool update();

    void render();
};
