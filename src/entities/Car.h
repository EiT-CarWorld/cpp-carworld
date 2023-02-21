#pragma once
#include <cstddef>
#include "raylib.h"
#include "driving/Route.h"
#include "driving/RouteFollower.h"
#include "carConfig.h"

#define NUM_CAR_MODELS 1
#define NUM_CAR_COLORS 6

class World;

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

    RouteFollower m_routeFollower;

    static const float LIDAR_ANGLES[NUM_LIDAR_ANGLES];
    static const float MIN_LIDAR_DISTANCE[NUM_LIDAR_ANGLES];
    static const float MAX_LIDAR_DIST;
    static const float MAX_CAR_ZONE_DIST;
    float m_lidarDistances[NUM_LIDAR_ANGLES]{};
    // Zone 0 is straight ahead, at angle -180/NUM_CAR_ZONE to 180/NUM_CAR_ZONE
    float m_carZoneDistances[NUM_CAR_ZONES]{};
    // The speed of the closest car in the zone, relative to our own speed
    // positive x direction is away from own car, positive z is towards previous zone in zone wheel
    Vector2 m_carZoneSpeed[NUM_CAR_ZONES]{};

    TurnInput m_turnInput{TURN_NO_TURN};
    GasInput m_gasInput{GAS_FREE};

    Vector3 m_position{};

    // in meters per second
    float m_speed{};
    // in radians, 0 is driving towards positive x, positive angle towards negative z
    float m_yaw{};
    // The angle of the front tires, multiplied by speed and applied every frame
    float m_yaw_speed{};

    bool m_crashed{};
public:
    explicit Car(Route* route);
    ~Car() = default;

    Vector3 getPosition();

    void takePlayerInput();
    void chooseAction(World* world);
    void chooseFreewheelAction();
    void update(World* world);
    void calculateSensors(World *world);
    Camera3D get3rdPersonCamera();
    bool hasFinishedRoute();
    bool hasCrashed();
    void render();
    void renderSensory();
    void renderHud();

private:
};
