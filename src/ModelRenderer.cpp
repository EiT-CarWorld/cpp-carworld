#include "ModelRenderer.h"
#include "config.h"
#include "rlgl.h"

Shader ModelRenderer::shader;
int ModelRenderer::ambientLightLoc;
int ModelRenderer::directionalLightLoc;
int ModelRenderer::directionalLightDirectionLoc;
int ModelRenderer::shaderModeLoc;

void ModelRenderer::loadStatic() {
    shader = LoadShader("res/shaders/base.vs", "res/shaders/model.fs");

    // raylib sets a bunch of these locs for us, but not /all/ the ones defined in SHADER_LOC_*
    shader.locs[SHADER_LOC_COLOR_SPECULAR] = GetShaderLocation(shader, "colSpecular");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "cameraPosition");

    ambientLightLoc = GetShaderLocation(shader, "ambientLight");
    directionalLightLoc = GetShaderLocation(shader, "directionalLight");
    directionalLightDirectionLoc = GetShaderLocation(shader, "directionalLightDirection");
    shaderModeLoc = GetShaderLocation(shader, "shaderMode");
    setMode(MODEL_MODE);
}

void ModelRenderer::unloadStatic() {
    UnloadShader(shader);
}

Model ModelRenderer::loadModel(char const *name) {
    Model model = LoadModel(name);
    for(int i = 0; i < model.materialCount; i++)
        model.materials[i].shader = shader;
    return model;
}

void ModelRenderer::unloadModel(Model model) {
    // This function doesn't unload any shaders or textures
    UnloadModel(model);
}

void ModelRenderer::setMode(ModelShaderMode mode) {
    SetShaderValue(shader, shaderModeLoc, &mode, SHADER_UNIFORM_INT);
}

ModelRenderer::ModelRenderer(Vector3 ambientLight, Vector3 directionalLight, Vector3 directionalLightDirection)
        : m_ambientLight(ambientLight), m_directionalLight(directionalLight), m_directionalLightDirection(directionalLightDirection) {}

ModelRenderer::~ModelRenderer() {}

void ModelRenderer::uploadState(Camera3D camera) {
    SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], &camera.position, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, ambientLightLoc, &m_ambientLight, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, directionalLightLoc, &m_directionalLight, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, directionalLightDirectionLoc, &m_directionalLightDirection, SHADER_UNIFORM_VEC3);
}