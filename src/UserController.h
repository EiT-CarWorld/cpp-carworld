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

    void lockMouse();
    void unlockMouse();
    void trySelectCar(Simulation* world);
public:
    void resetFreeCamera(Vector3 position);
    void updateSimulation(Simulation* world);
    Camera3D getCamera();
    void render(Simulation* world);
    void renderHUD(Simulation* world);
};
