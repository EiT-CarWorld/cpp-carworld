#include "Window.h"
#include "raylib.h"
#include "entities/FloorGrid.h"
#include "entities/ModelRenderer.h"
#include "entities/Node.h"

Window::Window(const char *title, int width, int height) {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(width, height, title);
    SetTargetFPS(60);
    SetExitKey(0); // Disable single key exit
    FloorGrid::loadStatic();
    ModelRenderer::loadStatic();
    Node::loadStatic();
}

Window::~Window() {
    Node::unloadStatic();
    ModelRenderer::unloadStatic();
    FloorGrid::unloadStatic();
    CloseWindow();
}

void Window::mainloop() {
    CameraController cameraController;
    cameraController.resetCamera({0, 30, 0});

    FloorGrid floorGrid({500, 500}, 1, BLUE);

    ModelRenderer renderer({0.4, 0.4, 0.5}, {0.7, 0.7, 0.6}, {1, -2, 1});

    PathNode pathNodes[4] = {{10, 0, 10},
                             {30, 0, 10},
                             {60, 0, 60},
                             {110, 0, 20}};
    Node start({0, 0, 0});
    Node end({140, 0, 20});
    Path path(pathNodes, 4);
    start.addOutPath(&path);
    end.addInPath(&path);

    while (!WindowShouldClose()) {
        // handle updates
        cameraController.updateCamera();
        Camera3D camera = cameraController.getCamera();

        // render frame
        BeginDrawing();

        ClearBackground(RAYWHITE);
        BeginMode3D(camera);
        floorGrid.render(camera);

        renderer.uploadState(camera);
        start.render();
        end.render();
        EndMode3D();

        DrawFPS(10, 10);
        EndDrawing();
    }
}