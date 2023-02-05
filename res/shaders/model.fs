#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0; // diffuse
uniform vec4 colDiffuse;
uniform sampler2D texture1; // specular / metalness
uniform vec4 colSpecular;

uniform vec3 cameraPosition;

uniform vec3 ambientLight;
uniform vec3 directionalLight;
uniform vec3 directionalLightDirection;

// Used to switch our "ubershader" between modes
uniform int shaderMode;

// Output fragment color
out vec4 finalColor;

void main() {
    vec4 diffuseColor = colDiffuse;
    vec3 specularColor = colSpecular.xyz;
    if (shaderMode == 0) {
        diffuseColor *= texture(texture0, fragTexCoord);
        specularColor *= texture(texture1, fragTexCoord).xyz;
    } else {
        vec4 textureValue1 = texture(texture0, fragPosition.xz/7);
        vec4 textureValue2 = texture(texture0, fragPosition.xz/11);
        diffuseColor *= (textureValue1+textureValue2)/2;
        vec3 texture1Value1 = texture(texture1, fragPosition.xz/7).xyz;
        vec3 texture1Value2 = texture(texture1, fragPosition.xz/11).xyz;
        specularColor *= (texture1Value1+texture1Value2)/2;
    }

    if (diffuseColor.w < 0.2)
        discard;

    vec3 lightSum = diffuseColor.xyz * ambientLight;

    // Do light calculations
    vec3 normal = normalize(fragNormal);
    vec3 dirLightDir = normalize(directionalLightDirection);

    // diffuse light from directional
    float directionalLightAngle = dot(normal, -dirLightDir);
    lightSum += diffuseColor.xyz * directionalLight * max(0, directionalLightAngle);

    float roughness = 0;

    // calculate specular against the directional light
    float reflectedLightAngle = dot(-dirLightDir, reflect(normalize(fragPosition-cameraPosition), normal));
    lightSum += specularColor * directionalLight * pow(max(0, reflectedLightAngle), 15-10*roughness);

    finalColor = vec4(lightSum, 1);
}