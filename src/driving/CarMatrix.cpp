#include "CarMatrix.h"
#include <cassert>
#include <algorithm>
#include <fstream>
#include "util.h"

CarMatrix::CarMatrix(size_t rows, size_t cols): rows(rows), cols(cols) {
    assert(rows > 0 && cols > 0);
    values = new float[rows*cols];
}

CarMatrix::CarMatrix(const CarMatrix &other) : rows(other.rows), cols(other.cols) {
    values = new float[rows*cols];
    std::copy(other.values, other.values+rows*cols, values);
}

CarMatrix::CarMatrix(CarMatrix &&other) : rows(other.rows), cols(other.cols), values(other.values) {
    other.values = nullptr;
}


CarMatrix& CarMatrix::operator=(CarMatrix &&other) {
    rows = other.rows;
    cols = other.cols;
    std::swap(values, other.values);
    return *this;
}

CarMatrix& CarMatrix::operator=(const CarMatrix& other) {
    *this = CarMatrix(other);
    return *this;
}

CarMatrix::~CarMatrix() {
    delete[] values;
}

size_t CarMatrix::getRows() {
    return rows;
}

size_t CarMatrix::getCols() {
    return cols;
}

float* CarMatrix::getValues() {
    return values;
}

void CarMatrix::multiply(const float *input, float *output) {
    for (int i = 0; i < cols; i++) {
        output[i] = 0;
        for (int j = 0; j < rows; j++)
            output[i] += input[j] * values[i*cols + j];
    }
}

void CarMatrix::mixIn(const CarMatrix &other, std::mt19937 random) {
    assert(cols == other.cols && rows == other.rows);
    std::uniform_int_distribution picker(0, 1);
    for(int i = 0; i < rows*cols; i++) {
        if( picker(random) == 0)
            values[i] = other.values[i];
    }
}

void CarMatrix::mutate(std::mt19937 random, float mutationChance) {
    std::uniform_real_distribution<float> chanceSample(0.f, 1.f);
    std::normal_distribution<float> mutationVal(0.f, 0.3f);
    for(int i = 0; i < rows*cols; i++) {
        if (chanceSample(random) < mutationChance)
            values[i] += mutationVal(random);
    }
}

void CarMatrix::saveToFile(std::ofstream& file) {
    WRITEOUT(file, (size_t)rows);
    WRITEOUT(file, (size_t)cols);
    for (int i = 0; i < rows*cols; i++)
        WRITEOUT(file, values[i]);
}

CarMatrix CarMatrix::loadFromFile(std::ifstream& file) {
    size_t rows, cols;
    READIN(file, rows);
    READIN(file, cols);
    CarMatrix result(rows, cols);
    for (int i = 0; i < rows*cols; i++) {
        READIN(file, result.values[i]);
    }
    return result;
}





