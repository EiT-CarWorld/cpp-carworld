#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

uniform vec4 colDiffuse;

uniform vec2 carPosition;
uniform float carYaw;
uniform float zoneLengths[16];

// Output fragment color
out vec4 finalColor;

void main() {
    int zoneCount = 16;
    // If the closest car in the zone is further away than this, we don't draw anything
    float maxDistanceBother = 40;

    vec2 difference = fragPosition.xz - carPosition;
    float angle = atan(-difference.y, difference.x);
    int zone = int( mod((angle-carYaw) / (2*3.1415926) * zoneCount + 0.5, zoneCount) );

    //if (zone != 12)
    //    return;

    float closestInZone = zoneLengths[zone];

    if (closestInZone > maxDistanceBother)
        discard;

    float distance = length(difference);
    if (distance > closestInZone)
        discard;

    finalColor = vec4(min(1, 5/closestInZone), max(0, 1-5/closestInZone), 0, 0.1);
}