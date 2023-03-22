#include "Car.h"
#include <cstdlib>
#include <cmath>
#include "Simulation.h"
#include "rendering/ModelRenderer.h"
#include "rendering/CarZonesVisualizer.h"
#include "carMath.h"
#include "carConfig.h"

Texture Car::diffuseTexture, Car::metalnessTexture;
Model Car::carModels[NUM_CAR_MODELS];
const Color Car::CAR_COLORS[NUM_CAR_COLORS] = {RED, GREEN, BLUE, WHITE, BLACK, YELLOW};

const float Car::LIDAR_ANGLES[NUM_LIDAR_ANGLES] = {-PI/2, -PI/4, 0, PI/4, PI/2};
// If we are closer than this to the road edge, we have crashed / driven off the road
const float Car::MIN_LIDAR_DISTANCE[NUM_LIDAR_ANGLES] = {0.9f, 1.3f, 1.6f, 1.3f, 0.9f};
const float Car::MAX_LIDAR_DIST = 100.f;

const float Car::MAX_CAR_ZONE_DIST = 100.0f;
const float Car::MIN_CAR_ZONE_DISTANCE[NUM_CAR_ZONES] = {2.f, 2.1f, 1.7f, 1.f, 1.f, 1.f, 1.7f, 2.1f,
                                                         2.f, 2.1f, 1.7f, 1.f, 1.f, 1.f, 1.7f, 2.1f};

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

Car::Car(const Route* route, float offset, CarBrain* brain): m_routeFollower(route), m_brain(brain) {
    m_color = CAR_COLORS[rand() % NUM_CAR_COLORS];
    m_modelNumber = rand() % NUM_CAR_MODELS;

    m_position = m_routeFollower.getStartNode()->position;
    m_position.y = 0.2;

    Vector3 startDirection =  m_routeFollower.getTarget()->position - m_position;
    m_yaw = atan2(-startDirection.z, startDirection.x);
    m_position.x -= offset * sinf(m_yaw);
    m_position.z += offset * cosf(m_yaw);
}

Vector3 Car::getPosition() {
    return m_position;
}

float Car::getScore() {
    return m_score;
}

void Car::chooseAction() {
    m_routeFollower.updateIfAtTarget(m_position);

    if (hasFinishedRoute()) {
        m_gasInput = GAS_FREE;
        m_turnInput = TURN_NO_TURN;
        return;
    }

    Vector3 distance = m_routeFollower.getTarget()->position - m_position;
    distance.y = 0;
    float direction = atan2(-distance.z, distance.x);
    float turn_offset = remainderf(m_yaw - direction, 2*PI);

    CarBrainInput input{};
    input.own_speed = m_speed;
    input.target_angle = turn_offset;
    input.target_distance = Vector3Length(distance);
    input.turn_in_target = m_routeFollower.getTurnInTarget();
    input.lidarData = &m_lidarDistances;
    input.carZoneDistances = &m_carZoneDistances;
    input.carZoneSpeeds = &m_carZoneSpeed;

    CarBrainOutput out = m_brain->takeAction(input);
    m_gasInput = out.gasInput;
    m_turnInput = out.turnInput;
}

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

void Car::chooseFreewheelAction() {
    m_gasInput = GAS_FREE;
}

// Used to calculate the distance to the rectangle bounds of other cars
static const float CAR_DIAGONAL = std::sqrt(CAR_WIDTH*CAR_WIDTH + CAR_LENGTH*CAR_LENGTH);
static const float CAR_DIAGONAL_ANGLE = std::atan2(CAR_WIDTH, CAR_LENGTH);
void Car::calculateSensors(Simulation* simulation) {
    if (m_crashed)
        return;

    // Calculate distances to the road edge
    for (int i = 0; i < NUM_LIDAR_ANGLES; i++) {
        float angle = m_yaw + LIDAR_ANGLES[i];
        Vector2 dir = {cosf(angle), sinf(angle)};
        m_lidarDistances[i] = simulation->getWorld()->getRayDistance({m_position.x, -m_position.z}, dir, MAX_LIDAR_DIST);
    }

    // Calculate distances to other cars. First set all distances to MAX
    for (int i = 0; i < NUM_CAR_ZONES; i++) {
        m_carZoneDistances[i] = MAX_CAR_ZONE_DIST;
        m_carZoneSpeed[i] = {0,0};
    }

    for (auto& other:simulation->getCars()) {
        if (other.get() == this) // Don't check our distance to ourselves
            continue;

        Vector2 difference { other->m_position.x - m_position.x, other->m_position.z - m_position.z};
        float distance = Vector2Length(difference);
        // The angle pointing from the center of this that to the center of the other car
        // Recall that 0 is positive x, positive angles go towards negative z
        float global_angle = atan2(-difference.y, difference.x);
        // The angle to the other car, relative to our forward angle
        float angle = global_angle - m_yaw;
        int zone = positive_mod((int)floor((angle / (2*PI)) * NUM_CAR_ZONES + 0.5f), NUM_CAR_ZONES);

        if (distance - CAR_DIAGONAL/2 >= m_carZoneDistances[zone])
            continue;

        // The other car's center is close enough that this might be the closest car in the zone
        // We pretend the other car is a rectangle, and let a ray between the car centers hit it.
        // Calculated based on the angle of the ray between car centers, and the angle of the other car.
        // Use remainder and abs to get value between 0 and PI/2, where 0 means we look right at the back of the other
        float otherCarAngle = abs(std::remainder(other->m_yaw - global_angle, PI));
        float otherCarExtent = (otherCarAngle < CAR_DIAGONAL_ANGLE)
                               ? CAR_LENGTH / cosf(otherCarAngle)
                               : CAR_WIDTH / cosf(PI/2-otherCarAngle);
        distance -= otherCarExtent/2;

        if (distance >= m_carZoneDistances[zone])
            continue;

        // We found a closer car!
        m_carZoneDistances[zone] = distance;
        // The relative speed difference, with positive x being speed directly away from us
        // And positive y means we speed towards the next zone
        float relative_angle = other->m_yaw - m_yaw;
        m_carZoneSpeed[zone] = {cosf(relative_angle) * other->m_speed - m_speed,
                                sinf(relative_angle) * other->m_speed}; //no need to add or sub, since orthogonal
    }
}

#define ACCELERATION 5.f
#define BASE_FRICTION 0.2f
// Scales linearly and quadratically with speed
#define FRICTION 0.0035f
#define QUADRATIC_FRICTION 0.0045f
// Used when the current speed is opposite direction of desired. Applied linearly
#define BREAKING_FRICTION 8.f //0.2f
// Friction applied to the car's speed due to turning. Quadratic
#define TURNING_SPEED_FRICTION 0.1f

#define TURN_SPEED 0.75f
// How quickly the wheels move when turning
#define TURN_FRICTION 0.006f
#define MAX_TURN 0.4f
// How much the max turning angle decreases per m/s speed
#define SPEED_MAX_TURN_PENALTY .009f

void Car::updatePhysics() {
    bool breaking = false;
    switch(m_gasInput) {
        case GAS_DRIVE:
            m_speed += ACCELERATION * SIM_DT;
            breaking = m_speed < 0;
            break;
        case GAS_REVERSE:
            m_speed -= ACCELERATION * SIM_DT;
            breaking = m_speed > 0;
            break;
        default: break;
    }

    float max_turn = MAX_TURN - abs(m_speed) * SPEED_MAX_TURN_PENALTY;

    switch (m_turnInput) {
        case TURN_LEFT:
            m_yaw_speed += TURN_SPEED * SIM_DT;
            m_yaw_speed -= m_yaw_speed * TURN_FRICTION * SIM_DT;
            m_yaw_speed = fmaxf(m_yaw_speed, 0);
            m_yaw_speed = fminf(m_yaw_speed, max_turn);
            break;
        case TURN_RIGHT:
            m_yaw_speed -= TURN_SPEED * SIM_DT;
            m_yaw_speed -= m_yaw_speed * TURN_FRICTION * SIM_DT;
            m_yaw_speed = fminf(m_yaw_speed, 0);
            m_yaw_speed = fmaxf(m_yaw_speed, -max_turn);
            break;
        default:
            m_yaw_speed -= sgn(m_yaw_speed) * fminf(TURN_SPEED * SIM_DT, abs(m_yaw_speed));
    }

    m_yaw += m_yaw_speed * m_speed * SIM_DT;
    m_yaw = std::remainder(m_yaw, 2*PI); // Keeping the yaw within +/- 180 degrees

    float friction = BASE_FRICTION + abs(m_speed) * (
            FRICTION +
            abs(m_speed) * (QUADRATIC_FRICTION + abs(m_yaw_speed) * TURNING_SPEED_FRICTION) +
            (float) breaking * BREAKING_FRICTION);
    m_speed -= sgn(m_speed) * friction * SIM_DT;
    if(abs(m_speed) > 0.05f) {
        m_position.x += m_speed * cosf(m_yaw) * SIM_DT;
        m_position.z += m_speed * -sinf(m_yaw) * SIM_DT;
    } else
        m_speed = 0.f;
}

void Car::update() {
    m_score -= SCORE_TIME_PENALTY * SIM_DT;

    if (m_crashed)
        return;

    // We can assume that chooseAction has been called before the call to update
    // so the lidar distances are updated
    for (int i = 0; i < NUM_LIDAR_ANGLES; i++) {
        if (m_lidarDistances[i] < MIN_LIDAR_DISTANCE[i]) {
            m_score -= SCORE_ROADSIDE_CRASH_PENALTY;
            m_crashed = true; break;
        }
    }
    for (int i = 0; i < NUM_CAR_ZONES; i++) {
        if (m_carZoneDistances[i] < MIN_CAR_ZONE_DISTANCE[i]) {
            m_crashed = true; break;
        }
    }

    if (m_crashed) {
        m_score -= SCORE_CRASH_PENALTY + SCORE_CRASH_SPEED_PENALTY * abs(m_speed);
        return;
    }

    float distanceToTarget = m_routeFollower.getDistanceToTarget2D(m_position);
    updatePhysics();
    float distanceImprovement = distanceToTarget - m_routeFollower.getDistanceToTarget2D(m_position);

    m_score += distanceImprovement * SCORE_GAIN_DISTANCE_COVER;
}

Camera3D Car::get3rdPersonCamera() {
    return Camera3D{
        .position = m_position + Vector3{-cosf(m_yaw)*20, 15, sinf(m_yaw)*20},
        .target = m_position,
        .up{0,1,0},
        .fovy = 80.0f,
        .projection = CAMERA_PERSPECTIVE
    };
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
}

void Car::renderSensory() {
    for (int i = 0; i < NUM_LIDAR_ANGLES; i++) {
        if(m_lidarDistances[i] < MAX_LIDAR_DIST) {
            float dist = m_lidarDistances[i];
            float angle = m_yaw + LIDAR_ANGLES[i];
            Vector3 pos = m_position + Vector3{0.f ,0.4f, 0.f};
            DrawLine3D(pos, {pos.x + cosf(angle)*dist, pos.y, pos.z-sinf(angle)*dist}, RED);
        }
    }

    m_routeFollower.getTarget()->renderCircle(RED);

    CarZonesVisualizer::DrawCarZones(m_position, m_yaw, m_carZoneDistances);
}

void Car::renderHud() {
    DrawText(TextFormat("%.0f km/h", m_speed*3.6f), GetScreenWidth()-240, GetScreenHeight()-40, 30, BLACK);
    DrawText(TextFormat("Score: %.0f", m_score), GetScreenWidth()-240, GetScreenHeight()-70, 30, BLACK);
}