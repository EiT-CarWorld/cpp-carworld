#pragma once
#include "carConfig.h"
#include "raylib.h"

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

struct CarBrainInput {
    // Our own speed
    float own_speed;
    // The relative angle to the center of the target
    float target_angle;
    // The distance to the center of the target
    float target_distance;

    // Sensor data from the car
    float (*lidarData)[NUM_LIDAR_ANGLES];
    float (*carZoneDistances)[NUM_CAR_ZONES];
    Vector2 (*carZoneSpeeds)[NUM_CAR_ZONES];
};
#define BRAIN_INPUT_LAYER_SIZE (3+NUM_LIDAR_ANGLES+NUM_CAR_ZONES*3)

struct CarBrainOutput {
    GasInput gasInput;
    TurnInput turnInput;
};

class CarBrain {



public:
    CarBrainOutput takeAction(CarBrainInput input);
};
