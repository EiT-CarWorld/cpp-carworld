#include "Window.h"
#include "raylib.h"
#include "entities/FloorGrid.h"
#include "ModelRenderer.h"
#include "entities/Node.h"
#include "entities/World.h"
#include "entities/Car.h"
#include "entities/Skybox.h"

Window::Window(const char *title, int width, int height) {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(width, height, title);
    SetTargetFPS(60);
    SetExitKey(0); // Disable single key exit
    FloorGrid::loadStatic();
    Skybox::loadStatic();
    ModelRenderer::loadStatic();
    Path::loadStatic();
    Car::loadStatic();
}

Window::~Window() {
    Car::unloadStatic();
    Path::unloadStatic();
    ModelRenderer::unloadStatic();
    Skybox::unloadStatic();
    FloorGrid::unloadStatic();
    CloseWindow();
}

void Window::mainloop() {
    CameraController cameraController;
    cameraController.resetCamera({0, 30, 0});

    FloorGrid floorGrid({500, 500}, 1, BLUE);
    World world;
    world.loadFromFile("res/maps/circuit.map");

    ModelRenderer renderer(
            {0.3, 0.3, 0.3},
            {0.7, 0.7, 0.7},
            {1, -2, 1});

    Car car({0, 0.2, 0});

    while (!WindowShouldClose()) {
        // handle updates
        cameraController.updateCamera();
        Camera3D camera = cameraController.getCamera();

        // render frame
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        Skybox::render(camera);
        floorGrid.render(camera);
        renderer.uploadState(camera);
        world.render();
        car.render();
        EndMode3D();

        DrawFPS(10, 10);
        EndDrawing();
    }
}