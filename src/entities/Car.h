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
    size_t m_modelNumber;
    Color m_color;

    Route* m_route;
    // Which path along the route we are on
    size_t m_route_path_index;
    // Which PathNode along the given path we are on
    size_t m_route_path_pathnode_index;

    Vector3 m_position;
    float m_speed;
    float m_yaw;

    bool m_player_controlled;

public:
    explicit Car(Route* route);
    ~Car() = default;
    void update(World* world);
    bool hasFinishedRoute();
    void render();
};
