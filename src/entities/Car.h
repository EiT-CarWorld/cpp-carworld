#pragma once
#include "raylib.h"

#define NUM_CAR_MODELS 1

class Car {
private:
    static Texture diffuseTexture, metalnessTexture;
    static Model carModels[NUM_CAR_MODELS];
public:
    static void loadStatic();
    static void unloadStatic();

private:
    Vector3 m_position;

public:
    Car(Vector3 position);
    ~Car() = default;
    void render();
};
