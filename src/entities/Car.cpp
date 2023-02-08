#include "Car.h"
#include <cstdlib>
#include <cmath>
#include <cassert>
#include "rendering/ModelRenderer.h"
#include "carMath.h"

Texture Car::diffuseTexture, Car::metalnessTexture;
Model Car::carModels[NUM_CAR_MODELS];
const Color Car::CAR_COLORS[NUM_CAR_COLORS] = {RED, GREEN, BLUE, WHITE, BLACK, YELLOW};

#include "rlgl.h"
void Car::loadStatic() {
    diffuseTexture = LoadTexture("res/models/cars/VehiclePack_BaseColor.png");
    metalnessTexture = LoadTexture("res/models/cars/VehiclePack_Metalnes.png");
    carModels[0] = ModelRenderer::loadModel("res/models/cars/Sedan_A_beige2.obj");
    carModels[0].materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = diffuseTexture;
    carModels[0].materials[1].maps[MATERIAL_MAP_SPECULAR].texture.id = rlGetTextureIdDefault();
    carModels[0].materials[1].maps[MATERIAL_MAP_SPECULAR].color = {200, 200, 200};

    carModels[0].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = diffuseTexture;
    carModels[0].materials[0].maps[MATERIAL_MAP_SPECULAR].texture.id = rlGetTextureIdDefault();
    carModels[0].materials[0].maps[MATERIAL_MAP_SPECULAR].color = {200, 200, 200};
}

void Car::unloadStatic() {
    ModelRenderer::unloadModel(carModels[0]);
    UnloadTexture(diffuseTexture);
    UnloadTexture(metalnessTexture);
}

Car::Car(Route* route): m_route(route) {
    m_color = CAR_COLORS[rand() % NUM_CAR_COLORS];
    m_modelNumber = rand() % NUM_CAR_MODELS;
    m_position = {10.f + (float)(rand() % 20)*10, 0.2, 10.f + (float)(rand() % 20)*10};
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

// If the car has no target, a target is given
// If the current target is reached, a new target is given
// If the route is over, and hasFinishedRoute() is true, target will be nullptr
void Car::findTarget() {
    if (m_target == nullptr) {
        if (hasFinishedRoute())
            return;

        if (m_route_path_pathnode_index == m_route->paths[m_route_path_index]->path_node_count) {
            m_target = m_route->nodes[m_route_path_index + 1];
        }
        else
            m_target = &m_route->paths[m_route_path_index]->path_nodes[m_route_path_pathnode_index];
    }

    // Check if we have reached the target
    float distance = Vector3Length(m_target->position - m_position);
    if (distance < m_target->diameter / 2) {
        // increase one of the two indecies
        if (m_route_path_pathnode_index < m_route->paths[m_route_path_index]->path_node_count)
            // We have more of the current Path to travel
            m_route_path_pathnode_index++;
        else {
            // We reached the Node at the end of the current path
            m_route_path_pathnode_index = 0;
            m_route_path_index++;
            if (m_route_path_index >= m_route->paths.size() && m_route->loops)
                m_route_path_index = 0;
        }

        m_target = nullptr;
        findTarget(); // Call again to get a new target
    }
}

void Car::chooseAction(World* world) {
    findTarget();

    if (hasFinishedRoute()) {
        m_gasInput = GAS_FREE;
        m_turnInput = TURN_NO_TURN;
        return;
    }

    m_gasInput = GAS_DRIVE;
    Vector3 distance = m_target->position - m_position;
    float direction = atan2(-distance.z, distance.x);
    float turn_offset = remainderf(m_yaw - direction, 2*PI);
    if(turn_offset < -.1)
        m_turnInput = TURN_LEFT;
    else if(turn_offset > .1)
        m_turnInput = TURN_RIGHT;
    else
        m_turnInput = TURN_NO_TURN;
}

void Car::update(World *world) {
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
}

void Car::followCamera(Camera* camera) {
    camera->position = m_position + Vector3{-cosf(m_yaw)*10, 5, sinf(m_yaw)*10};
    camera->target = m_position;
    camera->up = Vector3{0,1,0};
}

bool Car::hasFinishedRoute() {
    return m_route_path_index >= m_route->paths.size();
}

void Car::render() {
    carModels[m_modelNumber].materials[1].maps[MATERIAL_MAP_DIFFUSE].color = m_color;
    DrawModelEx(carModels[m_modelNumber], m_position, Vector3{0,1,0}, m_yaw*RAD2DEG, Vector3{1,1,1}, WHITE);
    if (m_target) {
        DrawCircle3D(m_target->position+Vector3{0,2,0}, m_target->diameter/2, Vector3{1,0,0}, 90.f, RED);
    }
}

void Car::renderHud() {
    DrawText(TextFormat("%.0f km/h", m_speed*60*3.6f), GetScreenWidth()-200, GetScreenHeight()-40, 30, BLACK);
}