#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// My own uniforms
uniform float gridSize;
uniform vec3 viewPos;
uniform vec3 gridColor;

// Output fragment color
out vec4 finalColor;

void main() {
    float grid_value = texture(texture0, (fragPosition.xz / gridSize)+vec2(0.5, 0.5)).x;

    // float cosViewAngle = dot(normalize(viewPos-fragPosition), normalize(fragNormal));

    finalColor = mix(colDiffuse, vec4(gridColor, 1), grid_value);
}