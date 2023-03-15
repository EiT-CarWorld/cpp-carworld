#pragma once
#include "Simulation.h"
#include "entities/Car.h"
#include "rendering/CameraController.h"
#include "GeneticSimulation.h"

enum class UserControllerMode {
    FREECAM, DRIVING
};

class UserController {
    GeneticSimulation* m_simulations;
    std::string m_configDir;

    bool m_mouseLock{false};
    UserControllerMode m_mode{UserControllerMode::FREECAM};
    Car* m_selectedCar{nullptr};
    CameraController m_cameraController;

    // Allows us to get the index of nodes by looking at them
    bool m_drawClosestNode{false};
    int m_closestNode; // Or -1 if no close node

    bool m_drawRoadBorders{false};
    bool m_drawCarSensors{false};
    bool m_freewheelAllCars{false};
    bool m_autoNextGeneration{false};

    void lockMouse();
    void unlockMouse();
    void updateRealtimeSimulation();
    void trySelectCar(Simulation* simulation);
    void drawClosestNode(Simulation* simulation);
    void makeSureSelectedCarExists(Simulation* simulation);
public:
    UserController(GeneticSimulation* simulations, std::string configDir);
    void resetFreeCamera(Vector3 position);
    void update();
    Camera3D getCamera();
    void render();
    void renderHUD();
};
