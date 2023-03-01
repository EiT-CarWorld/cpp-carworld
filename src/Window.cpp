#include "Window.h"
#include <algorithm>
#include "raylib.h"
#include "entities/FloorGrid.h"
#include "rendering/ModelRenderer.h"
#include "entities/Node.h"
#include "entities/World.h"
#include "entities/Car.h"
#include "rendering/CarZonesVisualizer.h"
#include "rendering/Skybox.h"
#include "UserController.h"
#include "carConfig.h"

Window::Window(const char *title, int width, int height) {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(width, height, title);
    SetTargetFPS(TARGET_FRAMERATE);
    SetExitKey(0); // Disable single key exit
    FloorGrid::loadStatic();
    Skybox::loadStatic();
    ModelRenderer::loadStatic();
    Path::loadStatic();
    Car::loadStatic();
    CarZonesVisualizer::loadStatic();
}

Window::~Window() {
    CarZonesVisualizer::unloadStatic();
    Car::unloadStatic();
    Path::unloadStatic();
    ModelRenderer::unloadStatic();
    Skybox::unloadStatic();
    FloorGrid::unloadStatic();
    CloseWindow();
}

void Window::mainloop() {
    FloorGrid floorGrid({1000, 1000}, 1, BLUE);

    ModelRenderer renderer(
            {0.3, 0.3, 0.3},
            {0.7, 0.7, 0.7},
            {1, -2, 1});

    // Create one initial brain
    std::vector<CarBrain> initial_brains;
    initial_brains.emplace_back(CarBrain::initializeMatrices(123, {20, 20}));

    GeneticSimulation simulations(std::move(initial_brains));
    simulations.setScoreOutputFile("out/scores.csv");

    UserController controller(&simulations, "res/config/");
    controller.resetFreeCamera({-10, 10, 40});

    while (!WindowShouldClose()) {
        controller.update();

        // render frame
        BeginDrawing();
        ClearBackground(RAYWHITE);

        Camera3D camera = controller.getCamera();
        BeginMode3D(camera);
        Skybox::render(camera);
        floorGrid.render(camera);
        renderer.uploadState(camera);
        controller.render();
        EndMode3D();

        controller.renderHUD();
        EndDrawing();
    }
    simulations.abortGeneration();
}