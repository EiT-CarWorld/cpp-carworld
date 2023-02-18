#include "Car.h"
#include <cstdlib>
#include <cmath>
#include "World.h"
#include "rendering/ModelRenderer.h"
#include "carMath.h"

Texture Car::diffuseTexture, Car::metalnessTexture;
Model Car::carModels[NUM_CAR_MODELS];
const Color Car::CAR_COLORS[NUM_CAR_COLORS] = {RED, GREEN, BLUE, WHITE, BLACK, YELLOW};

const float Car::LIDAR_ANGLES[NUM_LIDAR_ANGLES] = {-PI/2, -PI/4, 0, PI/4, PI/2};
const float Car::MIN_LIDAR_DISTANCE[NUM_LIDAR_ANGLES] = {0.9f, 1.3f, 1.6f, 1.3f, 0.9f};
const float Car::MAX_LIDAR_DIST = 100.f;
const float Car::MAX_CAR_ZONE_DIST = 100.0f;

#include "rlgl.h"
void Car::loadStatic() {
    diffuseTexture = LoadTexture("res/models/cars/VehiclePack_BaseColor.png");
    metalnessTexture = LoadTexture("res/models/cars/VehiclePack_Metalnes.png");
    carModels[0] = ModelRenderer::loadModel("res/models/cars/Sedan_A_beige2.obj");
    carModels[0].materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = diffuseTexture;
    carModels[0].materials[1].maps[MATERIAL_MAP_SPECULAR].texture.id = rlGetTextureIdDefault(); // A white texture
    carModels[0].materials[1].maps[MATERIAL_MAP_SPECULAR].color = {200, 200, 200};

    carModels[0].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = diffuseTexture;
    carModels[0].materials[0].maps[MATERIAL_MAP_SPECULAR].texture.id = rlGetTextureIdDefault(); // A white texture
    carModels[0].materials[0].maps[MATERIAL_MAP_SPECULAR].color = {200, 200, 200};
}

void Car::unloadStatic() {
    ModelRenderer::unloadModel(carModels[0]);
    UnloadTexture(diffuseTexture);
    UnloadTexture(metalnessTexture);
}

Car::Car(Route* route): m_routeFollower(route) {
    m_color = CAR_COLORS[rand() % NUM_CAR_COLORS];
    m_modelNumber = rand() % NUM_CAR_MODELS;

    m_position = m_routeFollower.getStartNode()->position;
    m_position.y = 0.2;

    Vector3 startDirection =  m_routeFollower.getTarget()->position - m_position;
    m_yaw = atan2(-startDirection.z, startDirection.x);
}

#define ACCELERATION 0.002f
#define FRICTION 0.0045f
#define FRICTION_POW 1.8f
#define TURNING_SPEED_FRICTION 0.03f

#define TURN_SPEED 0.01f
#define TURN_FRICTION 0.006f
#define MAX_TURN 0.3f
#define SPEED_MAX_TURN_PENALTY .4f

void Car::takePlayerInput() {
    if (IsKeyDown(KEY_UP))
        m_gasInput = GAS_DRIVE;
    else if (IsKeyDown(KEY_DOWN))
        m_gasInput = GAS_REVERSE;
    else
        m_gasInput = GAS_FREE;

    m_turnInput = TURN_NO_TURN;
    if(IsKeyDown(KEY_LEFT))
        m_turnInput = TURN_LEFT;
    else if(IsKeyDown(KEY_RIGHT))
        m_turnInput = TURN_RIGHT;
}

void Car::chooseAction(World* world) {
    m_routeFollower.updateIfAtTarget(m_position);

    if (hasFinishedRoute()) {
        m_gasInput = GAS_FREE;
        m_turnInput = TURN_NO_TURN;
        return;
    }

    m_gasInput = GAS_DRIVE;
    Vector3 distance = m_routeFollower.getTarget()->position - m_position;
    float direction = atan2(-distance.z, distance.x);
    float turn_offset = remainderf(m_yaw - direction, 2*PI);
    if(turn_offset < -.1)
        m_turnInput = TURN_LEFT;
    else if(turn_offset > .1)
        m_turnInput = TURN_RIGHT;
    else
        m_turnInput = TURN_NO_TURN;
}

void Car::calculateLIDAR(World* world) {
    for (int i = 0; i < NUM_LIDAR_ANGLES; i++) {
        float angle = m_yaw + LIDAR_ANGLES[i];
        Vector2 dir = {cosf(angle), -sinf(angle)};
        m_lidarDistances[i] = world->getRayDistance({m_position.x, m_position.z}, dir, MAX_LIDAR_DIST);
    }

    for (auto& other:world->getCars()) {
        Vector2 distance { other->m_position.x, other->m_position.z };
        float angle = std::remainder(atan2(distance.y, distance.x) - m_yaw, PI*2);

    }
}

void Car::update(World *world) {
    if (m_crashed)
        return;

    switch(m_gasInput) {
        case GAS_DRIVE: m_speed += ACCELERATION; break;
        case GAS_REVERSE: m_speed -= ACCELERATION; break;
        default: break;
    }

    float max_turn = MAX_TURN - abs(m_speed) * SPEED_MAX_TURN_PENALTY;

    switch (m_turnInput) {
        case TURN_LEFT:
        m_yaw_speed += TURN_SPEED;
        m_yaw_speed -= m_yaw_speed * TURN_FRICTION;
        m_yaw_speed = fmaxf(m_yaw_speed, 0);
        m_yaw_speed = fminf(m_yaw_speed, max_turn);
        break;
        case TURN_RIGHT:
        m_yaw_speed -= TURN_SPEED;
        m_yaw_speed -= m_yaw_speed * TURN_FRICTION;
        m_yaw_speed = fminf(m_yaw_speed, 0);
        m_yaw_speed = fmaxf(m_yaw_speed, -max_turn);
        break;
        default:
        m_yaw_speed -= sgn(m_yaw_speed) * fminf(TURN_SPEED, abs(m_yaw_speed));
    }

    m_speed -= m_speed * abs(m_yaw_speed) * TURNING_SPEED_FRICTION;
    m_yaw += m_yaw_speed * (m_speed > 1 ? powf(m_speed, 0.3) : m_speed);

    m_speed -= sgn(m_speed) * fmaxf(abs(m_speed), powf(abs(m_speed), FRICTION_POW)) * FRICTION;
    if(abs(m_speed) > 0.01f) {
        m_position.x += m_speed * cosf(m_yaw);
        m_position.z += m_speed * -sinf(m_yaw);
    }

    calculateLIDAR(world);

    for (int i = 0; i < NUM_LIDAR_ANGLES; i++) {
        if (m_lidarDistances[i] < MIN_LIDAR_DISTANCE[i])
            m_crashed = true;
    }
}

void Car::followCamera(Camera* camera) {
    camera->position = m_position + Vector3{-cosf(m_yaw)*10, 5, sinf(m_yaw)*10};
    camera->target = m_position;
    camera->up = Vector3{0,1,0};
}

bool Car::hasFinishedRoute() {
    return m_routeFollower.hasFinishedRoute();
}

bool Car::hasCrashed() {
    return m_crashed;
}

void Car::render() {
    carModels[m_modelNumber].materials[1].maps[MATERIAL_MAP_DIFFUSE].color = m_color;
    DrawModelEx(carModels[m_modelNumber], m_position,
                Vector3{0,1,0}, m_yaw*RAD2DEG, Vector3{1,1,1}, WHITE);

    for (int i = 0; i < NUM_LIDAR_ANGLES; i++) {
        if(m_lidarDistances[i] < MAX_LIDAR_DIST) {
            float dist = m_lidarDistances[i];
            float angle = m_yaw + LIDAR_ANGLES[i];
            DrawLine3D(m_position, {m_position.x + cosf(angle)*dist, m_position.y, m_position.z-sinf(angle)*dist}, RED);
        }
    }
}

void Car::renderHud() {
    DrawText(TextFormat("%.0f km/h", m_speed*60*3.6f), GetScreenWidth()-200, GetScreenHeight()-40, 30, BLACK);
}