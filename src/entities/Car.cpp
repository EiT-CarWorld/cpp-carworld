#include "Car.h"
#include <cstdlib>
#include "rendering/ModelRenderer.h"

Texture Car::diffuseTexture, Car::metalnessTexture;
Model Car::carModels[NUM_CAR_MODELS];
const Color Car::CAR_COLORS[NUM_CAR_COLORS] = {RED, GREEN, BLUE, WHITE, BLACK};

#include "rlgl.h"
void Car::loadStatic() {
    diffuseTexture = LoadTexture("res/models/cars/VehiclePack_BaseColor.png");
    metalnessTexture = LoadTexture("res/models/cars/VehiclePack_Metalnes.png");
    carModels[0] = ModelRenderer::loadModel("res/models/cars/Sedan_A_beige.obj");
    carModels[0].materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = diffuseTexture;
    carModels[0].materials[1].maps[MATERIAL_MAP_SPECULAR].texture.id = rlGetTextureIdDefault();
    carModels[0].materials[1].maps[MATERIAL_MAP_SPECULAR].color = {200, 200, 200};

    carModels[0].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = diffuseTexture;
    carModels[0].materials[0].maps[MATERIAL_MAP_SPECULAR].texture.id = rlGetTextureIdDefault();
    carModels[0].materials[0].maps[MATERIAL_MAP_SPECULAR].color = {200, 200, 200};
}

void Car::unloadStatic() {
    ModelRenderer::unloadModel(carModels[0]);
    UnloadTexture(diffuseTexture);
    UnloadTexture(metalnessTexture);
}

Car::Car(Route* route): m_route(route), m_speed() {
    m_color = CAR_COLORS[rand() % NUM_CAR_COLORS];
    m_modelNumber = rand() % NUM_CAR_MODELS;
    m_position = {10.f + (rand() % 20)*10, 0.5, 10.f + (rand() % 20)*10};
}

void Car::update(World *world) {

}

bool Car::hasFinishedRoute() {
    return false;
}

void Car::render() {
    carModels[0].materials[1].maps[MATERIAL_MAP_DIFFUSE].color = m_color;
    DrawModel(carModels[0], m_position, 1.0f, WHITE);
}