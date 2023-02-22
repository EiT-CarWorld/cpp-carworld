#pragma once
#include "Simulation.h"
#include "entities/Car.h"
#include "rendering/CameraController.h"

enum class UserControllerMode {
    FREECAM, DRIVING
};

class UserController {
    bool m_mouseLock{false};
    UserControllerMode m_mode{UserControllerMode::FREECAM};
    Car* m_selectedCar{nullptr};
    CameraController m_cameraController;

    bool m_drawRoadBorders{false};
    bool m_drawCarSensors{false};
    bool m_freewheelAllCars{false};
    bool m_removeDeadCars{false};

    // To make it very clear that rendering a simulation relies on also updating it
    // We always render the last simulation updated
    Simulation* m_lastUpdatedSimulation;

    void lockMouse();
    void unlockMouse();
    void trySelectCar();
    void makeSureSelectedCarExists();
public:
    void resetFreeCamera(Vector3 position);
    void updateSimulation(Simulation* simulation);
    Camera3D getCamera();
    void render();
    void renderHUD();
};
