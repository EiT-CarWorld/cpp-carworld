#include "Skybox.h"
#include "rendering/ModelRenderer.h"

Model Skybox::model;
Texture Skybox::texture;

void Skybox::loadStatic() {
    model = LoadModel("res/models/skybox.glb");
    texture = LoadTexture("res/textures/skybox.png");
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
}

void Skybox::unloadStatic() {
    UnloadModel(model);
    UnloadTexture(texture);
}

#include "rlgl.h"
void Skybox::render(Camera3D camera) {
    rlDisableDepthTest();
    DrawModel(model, camera.position, 1.0f, WHITE);
    rlEnableDepthTest();
}