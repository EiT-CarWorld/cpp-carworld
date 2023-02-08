#include "Window.h"
#include "raylib.h"
#include "entities/FloorGrid.h"
#include "rendering/ModelRenderer.h"
#include "entities/Node.h"
#include "World.h"
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
    world.createRoutes(1234, 1);

    ModelRenderer renderer(
            {0.3, 0.3, 0.3},
            {0.7, 0.7, 0.7},
            {1, -2, 1});

    while (!WindowShouldClose()) {
        // handle updates
        cameraController.updateCamera();
        Camera3D camera = cameraController.getCamera();

        while (world.getCarCount() < 100)
            world.spawnCar();
        world.updateCars();

        // render frame
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        Skybox::render(camera);
        floorGrid.render(camera);
        renderer.uploadState(camera);
        world.render();
        EndMode3D();

        DrawFPS(10, 10);
        EndDrawing();
    }
}