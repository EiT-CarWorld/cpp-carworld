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
    vec3 textureValue1 = texture(texture0, fragPosition.xz/7).xyz;
    vec3 textureValue2 = texture(texture0, fragPosition.xz/11).xyz;
    vec3 textureValue = (textureValue1+textureValue2)/2;
    float directionalLightAngle = dot(normalize(fragNormal), -normalize(directionalLightDirection));
    vec3 light = ambientLight + directionalLight * max(0, directionalLightAngle);
    finalColor = colDiffuse * vec4(textureValue * light, 1);
}