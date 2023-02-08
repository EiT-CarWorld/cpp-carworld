#pragma once
#include <cstddef>
#include "raylib.h"
#include "driving/Route.h"

#define NUM_CAR_MODELS 1
#define NUM_CAR_COLORS 6

class World;
class Car {
private:
    static Texture diffuseTexture, metalnessTexture;
    static Model carModels[NUM_CAR_MODELS];
    static const Color CAR_COLORS[NUM_CAR_COLORS];
public:
    static void loadStatic();
    static void unloadStatic();

private:
    size_t m_modelNumber{};
    Color m_color{};

    Route* m_route;
    // Which path along the route we are on
    size_t m_route_path_index{};
    // Which PathNode along the given path we are on
    // 0 means we target the 0th PathNode
    // if pathnode index == path_node_count, the Node ending the path is the target
    size_t m_route_path_pathnode_index{};
    // The current target, based on
    PathNode* m_target{};

    enum TurnInput {
        TURN_LEFT,
        TURN_RIGHT,
        TURN_NO_TURN
    };
    enum GasInput {
        GAS_DRIVE,
        GAS_REVERSE,
        GAS_FREE
    };
    TurnInput m_turnInput;
    GasInput m_gasInput;

    Vector3 m_position{};

    // in meters per frame (60fps target)
    float m_speed{};
    // in radians, 0 is positive x, positive angle towards negative z
    float m_yaw{};
    // only used for player controlled cars, for the time being
    float m_yaw_speed{};

public:
    explicit Car(Route* route);
    ~Car() = default;

    void takePlayerInput();
    void chooseAction(World* world);
    void update(World* world);
    void followCamera(Camera* camera);
    bool hasFinishedRoute();
    void render();
    void renderHud();

private:
    void findTarget();
};
