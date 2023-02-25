#include "CarMatrix.h"
#include <cassert>
#include <algorithm>

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




