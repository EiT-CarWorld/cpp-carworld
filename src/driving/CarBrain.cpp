#include "CarBrain.h"
#include <cassert>
#include <cmath>
#include <random>
#include <fstream>

CarBrain::CarBrain(std::vector<CarMatrix> matrices) : m_matrices(std::move(matrices)) {
    // Now we verify that the sizes of the matrices fit each other,
    // that the first and last matrices take correct input and output
    // and that no matrix is taller than the MAX_LAYER_SIZE
    // For each input, a 1-value is added to the end of the vector, to get a bias
    assert(!m_matrices.empty());
    assert(m_matrices.front().getCols() == BRAIN_INPUT_LAYER_SIZE + 1);
    assert(m_matrices.back().getRows() == BRAIN_OUTPUT_LAYER_SIZE);
    for (int i = 0; i+1 < m_matrices.size(); i++) {
        assert(m_matrices[i].getRows() < MAX_LAYER_SIZE);
        assert(m_matrices[i].getRows() + 1 == m_matrices[i+1].getCols());
    }
}

static float activation_function(float in) {
    if(in < 0)
        return in * 0.0f; // Change softness of ReLU
    return in;
}

static float last_activation_function(float in) {
    return 1.f / (1.f + expf(-in));
}

static float sigmoidPlus(float x, float scale) {
    return 2.f / (1 + exp(-x/scale)) - 1.f;
}

static float sigmoid(float x, float scale) {
    return 1.f / (1 + exp(-x/scale));
}

static_assert(BRAIN_INPUT_LAYER_SIZE <= MAX_LAYER_SIZE);
static thread_local float tmpVectorA[MAX_LAYER_SIZE+1], tmpVectorB[MAX_LAYER_SIZE+1];

CarBrainOutput CarBrain::takeAction(CarBrainInput input) {
    float *vectorIn = tmpVectorA, *vectorOut = tmpVectorB;

    // First we load up the input vector with sensor data
    int i = 0;
    vectorIn[i++] = input.own_speed / 100.f;
    vectorIn[i++] = input.target_angle;
    vectorIn[i++] = input.turn_in_target;
    vectorIn[i++] = input.target_distance / 100.f;
    for (int j = 0; j < NUM_LIDAR_ANGLES; j++)
        vectorIn[i++] = (*input.lidarData)[j] / 100.f;
    for (int j = 0; j < NUM_CAR_ZONES; j++) {
        vectorIn[i++] = (*input.carZoneDistances)[j] / 100.f;
        vectorIn[i++] = (*input.carZoneSpeeds)[j].x / 100.f;
        vectorIn[i++] = (*input.carZoneSpeeds)[j].y / 100.f;
    }
    assert(i == BRAIN_INPUT_LAYER_SIZE);

    // Then we do matrix multiplication and activation functions all the way
    for (int j = 0; j < m_matrices.size(); j++) {
        size_t inputs = m_matrices[j].getCols();
        if (j != 0) {
            std::swap(vectorIn, vectorOut);
            for (int k = 0; k < inputs-1; k++)
                vectorIn[k] = activation_function(vectorIn[k]);
        }
        vectorIn[inputs-1] = 1.0f;
        m_matrices[j].multiply(vectorIn, vectorOut);
    }
    // Finally the last activation function
    for (int k = 0; k < BRAIN_OUTPUT_LAYER_SIZE; k++)
        vectorOut[k] = last_activation_function(vectorOut[k]);

    // Use the activation of the last layer to drive car
    float gas  = vectorOut[0] - vectorOut[1];
    float turn = vectorOut[2] - vectorOut[3];
    return {
        gas > 0.1f ? GAS_DRIVE : gas < -0.1f ? GAS_REVERSE : GAS_FREE,
        turn > 0.1f ? TURN_LEFT : turn < -0.1f ? TURN_RIGHT : TURN_NO_TURN
    };
}

void CarBrain::setEvaluationScore(float score) {
    m_evaluationScore = score;
}

float CarBrain::getEvaluationScore() {
    return m_evaluationScore;
}

void CarBrain::mixIn(const CarBrain& other, std::mt19937& random) {
    assert(m_matrices.size() == other.m_matrices.size());
    for (int i = 0; i < m_matrices.size(); i++) {
        auto& matrix = m_matrices[i];
        auto& matrix2 = other.m_matrices[i];
        matrix.mixIn(matrix2, random);
    }
}

void CarBrain::mutate(std::mt19937& random, float mutateChance) {
    for (auto & matrix : m_matrices) {
        matrix.mutate(random, mutateChance);
    }
}

void CarBrain::saveToFile(std::ofstream &file) {
    file << m_matrices.size() << std::endl;
    // Each matrix is printed as its own line
    for (auto& m : m_matrices)
        m.saveToFile(file);
}

CarBrain CarBrain::loadFromFile(std::ifstream &file) {
    size_t N;
    file >> N;
    assert(file.good() && N > 0);
    std::vector<CarMatrix> matrices;
    for (size_t i = 0; i < N; i++) {
        assert (file.get() == '\n');
        matrices.emplace_back(CarMatrix::loadFromFile(file));
    }
    return CarBrain(std::move(matrices));
}

static CarMatrix initializeMatrix(std::mt19937& mt, size_t rows, size_t columns) {
    // Using He Weight Initialization
    float std = sqrtf(2.f / columns);
    std::normal_distribution<float> he_dist(0.f, std);
    CarMatrix matrix(rows, columns);
    float* values = matrix.getValues();
    for (int i = 0; i < rows * columns; i++)
        values[i] = he_dist(mt);
    return matrix;
}

std::vector<CarMatrix> CarBrain::initializeMatrices(
        unsigned long seed,
        std::vector<size_t> hidden_layer_sizes) {

    std::mt19937 mt(seed );

    hidden_layer_sizes.push_back(BRAIN_OUTPUT_LAYER_SIZE);
    std::vector<CarMatrix> matrices;
    size_t inputs = BRAIN_INPUT_LAYER_SIZE;
    for (size_t outputs : hidden_layer_sizes) {
        matrices.emplace_back(initializeMatrix(mt, outputs, inputs+1));
        inputs = outputs;
    }
    return matrices;
}
