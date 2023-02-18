#pragma once

#include "raylib.h"

class Skybox {
private:
    static Model model;
    static Texture texture;
public:
    static void loadStatic();
    static void unloadStatic();
    static void render(Camera3D camera);
};