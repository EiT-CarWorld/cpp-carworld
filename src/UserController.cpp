#include "UserController.h"
#include "raylib.h"
#include "rlgl.h"
#include <cassert>
#include "util.h"
#include "tinyfiledialogs.h"

UserController::UserController(GeneticSimulation* simulations, std::string configDir)
        : m_simulations(simulations), m_configDir(std::move(configDir)) {}

void UserController::resetFreeCamera(Vector3 position) {
    m_cameraController.resetCamera(position);
}

void UserController::updateRealtimeSimulation() {
    Simulation* simulation = m_simulations->getRealtimeSimulation();
    if (!simulation) {
        m_selectedCar = nullptr;
        m_mode = UserControllerMode::FREECAM;
        return;
    }

    // Does things like spawning new cars, or freezing the score if enough frames have passed
    m_simulations->preSimulationFrame(simulation);

    // First we remove cars that have crashed, or finished their route
    // Making sure to automatically deselect removed cars
    auto& cars = simulation->getCars();

    makeSureSelectedCarExists(simulation);

    // Now we handle keypresses
    if (m_mouseLock && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && m_mode == UserControllerMode::FREECAM)
        trySelectCar(simulation);
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
    if (IsKeyPressed(KEY_C))
        m_drawClosestNode = !m_drawClosestNode;
    if (IsKeyPressed(KEY_N))
        simulation->spawnCar(0, 0.f); // Use route 0, no offset

    // If something has caused our selected car to be deselected, or it no longer exists
    if (m_selectedCar == nullptr && m_mode == UserControllerMode::DRIVING)
        m_mode = UserControllerMode::FREECAM;

    // Let all cars decide on their action
    simulation->takeCarActions();

    if (m_freewheelAllCars)
        for(auto& car:simulation->getCars())
            car->chooseFreewheelAction();

    // If we are controlling a car, manually override the AIs action
    if (m_mode == UserControllerMode::DRIVING) {
        assert(m_selectedCar);
        m_selectedCar->takePlayerInput();
    }

    // Finally do the actual update
    simulation->updateCars();
}

void UserController::update() {
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

    if (IsKeyPressed(KEY_T))
        m_autoNextGeneration = !m_autoNextGeneration;

    if (m_simulations->hasGenerationRunning()) {
        if (m_simulations->getSimulationsRunning() == 0 && (IsKeyPressed(KEY_ENTER) || m_autoNextGeneration)) {
            m_simulations->finishGeneration();
            m_selectedCar = nullptr;
        }
        else if (IsKeyPressed(KEY_BACKSPACE)) {
            m_simulations->abortGeneration();
            m_autoNextGeneration = false;
            m_selectedCar = nullptr;
        }
    }

    // If no generation is running, we can make changes and start next generation
    if (!m_simulations->hasGenerationRunning()) {
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
            unlockMouse();
            const char* filetypes[] = {"*.gen"};
            char const *dest = tinyfd_saveFileDialog(
                    "Save gene pool","res/brains/",1,filetypes,"Gene pool file");
            if (dest != NULL)
                m_simulations->saveGenePool(dest);
        }
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_O)) {
            unlockMouse();
            const char* filetypes[] = {"*.gen"};
            char const *dest = tinyfd_openFileDialog(
                    "Load gene pool","res/brains/",1,filetypes,"Gene pool file", false);
            if (dest != NULL)
                m_simulations->saveGenePool(dest);
        }

        if (IsKeyPressed(KEY_R) || m_autoNextGeneration) {

            // If there exists a config file for this generation, run it first
            char configFileName[100];
            snprintf(configFileName, sizeof(configFileName),
                     "%s/%ld.txt", m_configDir.c_str(), m_simulations->getGenerationNumber());
            if ( fileExists(configFileName) ) {
                bool loadResult = m_simulations->loadParameterFile(configFileName);
                if (!loadResult) { // If something in the file fails, wait here for a human to fix it
                    m_autoNextGeneration = false;
                    return;
                }
            }

            m_simulations->startParallelGeneration(!m_autoNextGeneration);
        }
    }

    // Is a NO-OP if there is no realtime simulation running
    updateRealtimeSimulation();

    // Update the current camera mode
    if (m_mouseLock && m_mode == UserControllerMode::FREECAM)
        m_cameraController.updateCamera();
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
    Simulation* simulation = m_simulations->getRealtimeSimulation();
    if (!simulation)
        return;

    simulation->render();

    if (m_drawClosestNode)
        drawClosestNode(simulation);

    if (m_drawRoadBorders)
        simulation->getWorld()->renderRoadBorders();

    if (m_drawCarSensors) {
        if (m_selectedCar)
            m_selectedCar->renderSensory();
        else {
            rlDisableDepthMask();
            for (auto& car:simulation->getCars())
                car->renderSensory();
            rlEnableDepthMask();
        }
    }
}

void UserController::renderHUD() {
    DrawFPS(10, 10);

    if (m_mode == UserControllerMode::FREECAM && m_mouseLock)
        DrawRectangle(GetRenderWidth()/2, GetRenderHeight()/2, 2, 2, WHITE);

    int y = 0;
#define DRAW_LINE(text) DrawText((text), 10, 30+(y++)*20, 20, BLACK)
#define DRAW_TOGGLE(text, state) DRAW_LINE(TextFormat((text), (state)?'X':' '))

    DRAW_LINE(TextFormat("Generation %d", m_simulations->getGenerationNumber()));
    DRAW_TOGGLE("T - Auto next generation (%c)", m_autoNextGeneration);
    if (m_simulations->hasGenerationRunning()) {
        DRAW_LINE(TextFormat("Simulations left: %d", m_simulations->getSimulationsRunning()));
        if (m_simulations->getSimulationsRunning() == 0)
            DRAW_LINE(TextFormat("Enter - Finish Generation"));
        DRAW_LINE(TextFormat("Backspace - Cancel Generation"));
    } else {
        DRAW_LINE(TextFormat("R - Start generation"));
    }

    // The rest of this function is only run when a simulation is being run in realtime
    Simulation* simulation = m_simulations->getRealtimeSimulation();
    if (!simulation)
        return;

    DRAW_LINE("=== Realtime simulation ===");
    DRAW_LINE(TextFormat("Frame %d/%d", simulation->getFrameNumber(), m_simulations->getFramesPerSimulation()));

    DRAW_LINE(TextFormat("N - spawn car (%d)", simulation->getCars().size()));
    DRAW_TOGGLE("L - toggle lines (%c)", m_drawRoadBorders);
    DRAW_TOGGLE("V - toggle sensor view (%c)", m_drawCarSensors);
    if (m_selectedCar) {
        DRAW_LINE("K - drive selected car");
        DRAW_LINE("RMB - deselect car");
    } else {
        DRAW_LINE("LMB - Select car");
    }
    DRAW_TOGGLE("C - highlight closest node (%c)", m_drawClosestNode);
    if (m_drawClosestNode) {
        DRAW_LINE(TextFormat("Closest node: %d", m_closestNode));
    }
    DRAW_TOGGLE("F - freewheel (%c)", m_freewheelAllCars);
    float totalScore = simulation->getTotalSimulationScore();
    DRAW_LINE(TextFormat("Total score: %.0f", totalScore));
#undef DRAW_LINE
#undef DRAW_TOGGLE

    if (m_selectedCar)
        m_selectedCar->renderHud();
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
void UserController::trySelectCar(Simulation* simulation) {
    assert(m_mode == UserControllerMode::FREECAM);

    Camera3D camera = m_cameraController.getCamera();
    Vector3 lookingDirection = camera.target-camera.position;
    if (lookingDirection.y > -0.1)
        return; // Only work if we are actually looking down

    // We target the plane 0.6f above ground
    float floorDist = (camera.position.y - 0.6f) / (-lookingDirection.y);
    Vector3 floorHit = camera.position + floorDist * lookingDirection;
    for (auto& car: simulation->getCars()) {
        Vector3 difference = car->getPosition() - floorHit;
        difference.y = 0; // Only care about distance in XZ plane
        float distance = Vector3Length(difference);
        if (distance < CAR_LENGTH) {
            m_selectedCar = car.get();
            break;
        }
    }
}

void UserController::drawClosestNode(Simulation* simulation) {
    Camera3D camera = m_cameraController.getCamera();
    Vector3 lookingDirection = camera.target-camera.position;
    if (lookingDirection.y > -0.1)
        return;
    // We target the ground plane

    float floorDist = camera.position.y / -lookingDirection.y;
    Vector3 floorHit = camera.position + floorDist * lookingDirection;
    auto& nodes = simulation->getWorld()->getNodes();

    m_closestNode = -1;
    float closest = 100.0f;
    for (int i = 0; i < nodes.size(); i++) {
        float dist = Vector3Length(floorHit - nodes[i].position);
        if (dist < closest) {
            closest = dist;
            m_closestNode = i;
        }
    }

    if (m_closestNode != -1) {
        nodes[m_closestNode].renderCircle(ORANGE);
    }
}

// The selected car could have been removed for some reason.
// Therefore we need to check that the car we have selected, still exists
void UserController::makeSureSelectedCarExists(Simulation* simulation) {
    for (auto& car:simulation->getCars())
        if (car.get() == m_selectedCar)
            return;
    m_selectedCar = nullptr;
}