#include "ModelRenderer.h"
#include "config.h"
#include "rlgl.h"

Shader ModelRenderer::shader;
int ModelRenderer::ambientLightLoc;
int ModelRenderer::directionalLightLoc;
int ModelRenderer::directionalLightDirectionLoc;

void ModelRenderer::loadStatic() {
    shader = LoadShader("res/shaders/base.vs", "res/shaders/model.fs");
    ambientLightLoc = GetShaderLocation(shader, "ambientLight");
    directionalLightLoc = GetShaderLocation(shader, "directionalLight");
    directionalLightDirectionLoc = GetShaderLocation(shader, "directionalLightDirection");
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

ModelRenderer::ModelRenderer(Vector3 ambientLight, Vector3 directionalLight, Vector3 directionalLightDirection)
        : m_ambientLight(ambientLight), m_directionalLight(directionalLight), m_directionalLightDirection(directionalLightDirection) {}

ModelRenderer::~ModelRenderer() {}

void ModelRenderer::uploadState(Camera3D camera) {
    SetShaderValue(shader, ambientLightLoc, &m_ambientLight, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, directionalLightLoc, &m_directionalLight, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, directionalLightDirectionLoc, &m_directionalLightDirection, SHADER_UNIFORM_VEC3);
}