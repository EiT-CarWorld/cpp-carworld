#pragma once
#include <vector>
#include <random>
#include "carConfig.h"
#include "raylib.h"
#include "CarMatrix.h"

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
#define BRAIN_OUTPUT_LAYER_SIZE 4

struct CarBrainOutput {
    GasInput gasInput;
    TurnInput turnInput;
};

#define MAX_LAYER_SIZE 128
class CarBrain {
    std::vector<CarMatrix> m_matrices;
    // The score given to this brain, by a simulation running it
    float m_evaluationScore{};
public:
    explicit CarBrain(std::vector<CarMatrix> matrices);
    CarBrainOutput takeAction(CarBrainInput input);
    void setEvaluationScore(float score);
    float getEvaluationScore();

    void mixIn(const CarBrain& other, std::mt19937& random);
    void mutate(std::mt19937& random);

    // Creates an array of matrices between the layers in a brain
    static std::vector<CarMatrix> initializeMatrices(
            unsigned long seed,
            std::vector<size_t> hidden_layer_sizes);
};


