#include "ParticleEffect.h"
#include <cmath>

ParticleEffect::ParticleEffect(Vector3 position, Color color, size_t maxAge, bool eternal, float maxRadius)
        : m_position(position), m_color(color), m_maxAge(maxAge), m_eternal(eternal), m_maxRadius(maxRadius) {}

// Returns false if the particle effect should be removed
bool ParticleEffect::update() {
    m_framesLived++;
    return (m_framesLived < m_maxAge) || m_eternal;
}

void ParticleEffect::render() {
    Color color = m_color;
    float radius = m_maxRadius;

    // First third is expanding
    if (m_framesLived*3 < m_maxAge)
        radius *= (m_framesLived*3.f / m_maxAge);
    if (m_framesLived < m_maxAge)
        color.a = (unsigned char)(255.f * std::fminf(0.7f, 1.0f-(float)m_framesLived/m_maxAge));
    else {
        radius = 2.0f;
        color.a = 200;
    }
    DrawSphere(m_position, radius, color);
}