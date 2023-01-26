#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec3 ambientLight;
uniform vec3 directionalLight;
uniform vec3 directionalLightDirection;

// Output fragment color
out vec4 finalColor;

void main() {
    vec3 textureValue = texture(texture0, fragPosition.xz/3).xyz;
    float directionalLightAngle = dot(normalize(fragNormal), -normalize(directionalLightDirection));
    vec3 light = ambientLight + directionalLight * max(0, directionalLightAngle);
    finalColor = colDiffuse * vec4(textureValue * light, 1);
}