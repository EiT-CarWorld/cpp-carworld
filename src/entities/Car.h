#pragma once
#include <cstddef>
#include "raylib.h"
#include "driving/Route.h"
#include "driving/RouteFollower.h"
#include "driving/CarBrain.h"
#include "carConfig.h"

#define NUM_CAR_MODELS 1
#define NUM_CAR_COLORS 6

class Simulation;

class Car {
private:
    static Texture diffuseTexture, metalnessTexture;
    static Model carModels[NUM_CAR_MODELS];
    static const Color CAR_COLORS[NUM_CAR_COLORS];

    static const float LIDAR_ANGLES[NUM_LIDAR_ANGLES];
    static const float MIN_LIDAR_DISTANCE[NUM_LIDAR_ANGLES];
    static const float MAX_LIDAR_DIST;

    static const float MIN_CAR_ZONE_DISTANCE[NUM_CAR_ZONES];
    static const float MAX_CAR_ZONE_DIST;
public:
    static void loadStatic();
    static void unloadStatic();

private:
    size_t m_modelNumber{};
    Color m_color{};

    RouteFollower m_routeFollower;

    float m_lidarDistances[NUM_LIDAR_ANGLES]{};
    // Zone 0 is straight ahead, at angle -180/NUM_CAR_ZONE to 180/NUM_CAR_ZONE
    float m_carZoneDistances[NUM_CAR_ZONES]{};
    // The speed of the closest car in the zone, relative to our own speed
    // positive x direction is away from own car, positive y is orthogonal speed towards the next zone
    Vector2 m_carZoneSpeed[NUM_CAR_ZONES]{};

    CarBrain* m_brain;

    TurnInput m_turnInput{TURN_NO_TURN};
    GasInput m_gasInput{GAS_FREE};

    Vector3 m_position{};
    // in meters per second
    float m_speed{};
    // in radians, 0 is driving towards positive x, positive angle towards negative z
    float m_yaw{};
    // The angle of the front tires, multiplied by speed and applied every frame
    float m_yaw_speed{};

    float m_score{SCORE_INITIAL_SCORE};
    bool m_crashed{false};

public:
    explicit Car(const Route* route, float offset, CarBrain* brain);
    ~Car() = default;
    Vector3 getPosition();
    float getScore();

    void chooseAction();
    void takePlayerInput();
    void chooseFreewheelAction();
    void update();
    void calculateSensors(Simulation* simulation);
    Camera3D get3rdPersonCamera();
    bool hasFinishedRoute();
    bool hasCrashed();
    void render();
    void renderSensory();
    void renderHud();

private:
    void updatePhysics();
};
