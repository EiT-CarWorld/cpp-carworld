#include "UserController.h"
#include "raylib.h"
#include "rlgl.h"
#include <cassert>

void UserController::resetFreeCamera(Vector3 position) {
    m_cameraController.resetCamera(position);
}

void UserController::updateSimulation(Simulation* simulation) {
    m_lastUpdatedSimulation = simulation;

    // First we remove cars that have crashed, or finished their route
    // Making sure to automatically deselect removed cars
    auto& cars = simulation->getCars();

    if (m_removeDeadCars) {
        for (int i = 0; i < cars.size(); i++) {
            if (cars[i]->hasCrashed() || cars[i]->hasFinishedRoute()) {
                cars[i].swap(cars.back());
                cars.pop_back();
                i--;
            }
        }
    }

    makeSureSelectedCarExists();

    // Then we check if there are any reasons to change mode
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        lockMouse();
    // Pressing ESC leaves driving, or leaves mouse lock if not driving
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (m_mode == UserControllerMode::DRIVING)
            m_mode = UserControllerMode::FREECAM;
        else
            unlockMouse();
    }
    // Unfocusing is another way of unlocking the mouse, without leaving driving
    if (!IsWindowFocused())
        unlockMouse();

    // Now we handle keypresses
    if (m_mouseLock && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && m_mode == UserControllerMode::FREECAM)
        trySelectCar();
    if (IsKeyPressed(KEY_K) && m_selectedCar)
        m_mode = UserControllerMode::DRIVING;
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        m_selectedCar = nullptr;
    if (IsKeyPressed(KEY_L))
        m_drawRoadBorders = !m_drawRoadBorders;
    if (IsKeyPressed(KEY_V))
        m_drawCarSensors = !m_drawCarSensors;
    if (IsKeyPressed(KEY_F))
        m_freewheelAllCars = !m_freewheelAllCars;
    if (IsKeyPressed(KEY_N))
        simulation->spawnCar();

    // If something has caused our selected car to be deselected, or it no longer exists
    if (m_selectedCar == nullptr && m_mode == UserControllerMode::DRIVING)
        m_mode = UserControllerMode::FREECAM;

    // Update the current camera mode
    if (m_mouseLock && m_mode == UserControllerMode::FREECAM)
        m_cameraController.updateCamera();

    // Let all cars decide on their action
    simulation->takeCarActions();

    if (m_freewheelAllCars)
        for(auto& car:cars)
            car->chooseFreewheelAction();

    // If we are controlling a car, manually override the AIs action
    if (m_mode == UserControllerMode::DRIVING) {
        assert(m_selectedCar);
        m_selectedCar->takePlayerInput();
    }

    // Finally do the actual update
    simulation->updateCars();
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

void UserController::render() {
    assert(m_lastUpdatedSimulation);
    m_lastUpdatedSimulation->render();

    if (m_drawRoadBorders)
        m_lastUpdatedSimulation->getWorld()->renderRoadBorders();

    if (m_drawCarSensors) {
        if (m_selectedCar)
            m_selectedCar->renderSensory();
        else {
            rlDisableDepthMask();
            for (auto& car:m_lastUpdatedSimulation->getCars())
                car->renderSensory();
            rlEnableDepthMask();
        }
    }
}

void UserController::renderHUD() {
    DrawFPS(10, 10);
    int y = 0;
#define DRAW_LINE(text) DrawText((text), 10, 30+(y++)*20, 20, BLACK)
#define DRAW_TOGGLE(text, state) DRAW_LINE(TextFormat((text), (state)?'X':' '))
    DRAW_LINE(TextFormat("N - spawn car (%d)", m_lastUpdatedSimulation->getCars().size()));
    DRAW_TOGGLE("L - toggle lines (%c)", m_drawRoadBorders);
    DRAW_TOGGLE("V - toggle sensor view (%c)", m_drawCarSensors);
    if (m_selectedCar) {
        DRAW_LINE("K - drive selected car");
        DRAW_LINE("RMB - deselect car");
    } else {
        DRAW_LINE("LMB - Select car");
    }
    DRAW_TOGGLE("F - freewheel (%c)", m_freewheelAllCars);
    float totalScore = m_lastUpdatedSimulation->getTotalSimulationScore();
    DRAW_LINE(TextFormat("Total score: %.0f", totalScore));
#undef DRAW_LINE
#undef DRAW_TOGGLE

    if (m_selectedCar)
        m_selectedCar->renderHud();

    if (m_mode == UserControllerMode::FREECAM && m_mouseLock)
        DrawRectangle(GetRenderWidth()/2, GetRenderHeight()/2, 2, 2, WHITE);
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

// Uses the center of the camera, and camera direction, to shoot a ray at the ground, and select the car there, if any
void UserController::trySelectCar() {
    assert(m_mode == UserControllerMode::FREECAM);
    assert(m_lastUpdatedSimulation);

    Camera3D camera = m_cameraController.getCamera();
    Vector3 lookingDirection = camera.target-camera.position;
    if (lookingDirection.y < -0.1) {
        // Only work if we are actually looking down
        // We target the plane 0.6f above ground
        float floorDist = (camera.position.y - 0.6f) / (-lookingDirection.y);
        Vector3 floorHit = camera.position + floorDist * lookingDirection;
        for (auto& car: m_lastUpdatedSimulation->getCars()) {
            Vector3 difference = car->getPosition() - floorHit;
            difference.y = 0; // Only care about distance in XZ plane
            float distance = Vector3Length(difference);
            if (distance < CAR_LENGTH) {
                m_selectedCar = car.get();
                break;
            }
        }
    }
}

// The Simulation being passed to update can suddenly be replaced by a new simulation,
// Or the selected car could have been removed for some reason.
// Therefore we need to check that the car we have selected, still exists
void UserController::makeSureSelectedCarExists() {
    assert(m_lastUpdatedSimulation);
    for (auto& car:m_lastUpdatedSimulation->getCars())
        if (car.get() == m_selectedCar)
            return;
    m_selectedCar = nullptr;
}