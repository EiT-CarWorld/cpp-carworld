#include "UserController.h"
#include "raylib.h"
#include "rlgl.h"
#include <cassert>

void UserController::resetFreeCamera(Vector3 position) {
    m_cameraController.resetCamera(position);
}

void UserController::updateWorld(World* world) {
    // First we remove cars that have crashed, or finished their route
    // Making sure to automatically deselect removed cars
    auto& cars = world->getCars();
    for (int i = 0; i < cars.size(); i++) {
        if (cars[i]->hasCrashed() || cars[i]->hasFinishedRoute()) {
            if (cars[i].get() == m_selectedCar)
                m_selectedCar = nullptr;
            cars[i].swap(cars.back());
            cars.pop_back();
            i--;
        }
    }

    // If the selected car no longer exists, go back to freecam
    if (m_selectedCar == nullptr && m_mode == UserControllerMode::DRIVING)
        m_mode = UserControllerMode::FREECAM;

    // Then we check if there are any reasons to change mode
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        lockMouse();
    // Pressing ESC leaves driving, or leaves mouse lock if not driving
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (m_mode == UserControllerMode::DRIVING) {
            m_mode = UserControllerMode::FREECAM;
        } else {
            unlockMouse();
        }
    }
    // Unfocusing is another way of unlocking the mouse, without leaving driving
    if (!IsWindowFocused())
        unlockMouse();

    // Now we handle keypresses
    if (IsKeyPressed(KEY_K) && !cars.empty() && m_mode != UserControllerMode::DRIVING) {
        m_mode = UserControllerMode::DRIVING;
        m_selectedCar = world->getCars()[0].get();
    }
    if (IsKeyPressed(KEY_L))
        m_drawRoadBorders = !m_drawRoadBorders;
    if (IsKeyPressed(KEY_V))
        m_drawCarSensors = !m_drawCarSensors;
    if (IsKeyPressed(KEY_N))
        world->spawnCar();

    // Update the current camera mode
    if (m_mouseLock && m_mode == UserControllerMode::FREECAM)
        m_cameraController.updateCamera();

    // Let all cars decide on their action
    world->takeCarActions();

    // If we are controlling a car, manually override the AIs action
    if (m_mode == UserControllerMode::DRIVING) {
        assert(m_selectedCar);
        m_selectedCar->takePlayerInput();
    }

    // Finally do the actual update
    world->updateCars();
}

Camera3D UserController::getCamera() {
    switch(m_mode) {
        case UserControllerMode::FREECAM: return m_cameraController.getCamera();
        case UserControllerMode::DRIVING:
            assert(m_selectedCar);
            return m_selectedCar->get3rdPersonCamera();
        default: assert(false);
    }
}

void UserController::renderWorld(World* world) {
    world->render();

    if (m_drawRoadBorders)
        world->renderRoadBorders();

    if (m_drawCarSensors) {
        if (m_selectedCar)
            m_selectedCar->renderSensory();
        else {
            rlDisableDepthMask();
            for (auto& car:world->getCars())
                car->renderSensory();
            rlEnableDepthMask();
        }
    }
}

void UserController::renderHUD(World* world) {
    if (m_selectedCar)
        m_selectedCar->renderHud();
    size_t carCount = world->getCars().size();
    DrawFPS(10, 10);
    DrawText(TextFormat("N - spawn car (%d)", carCount), 10, 30, 20, BLACK);
    DrawText("L - toggle lines", 10, 50, 20, BLACK);
    DrawText("V - toggle sensor view", 10, 70, 20, BLACK);
    if (carCount > 0)
        DrawText("K - drive a car", 10, 90, 20, BLACK);
}

void UserController::lockMouse() {
    if (m_mouseLock)
        return;
    DisableCursor();
    m_mouseLock = true;
}

void UserController::unlockMouse() {
    if (!m_mouseLock)
        return;
    EnableCursor();
    m_mouseLock = false;
}