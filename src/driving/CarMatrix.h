#pragma once
#include <cstddef>
#include <random>

struct CarMatrix {
    size_t rows, cols;
    float *values;

    CarMatrix(size_t rows, size_t cols);
    CarMatrix(const CarMatrix& other);
    CarMatrix(CarMatrix&& other);
    CarMatrix& operator= (const CarMatrix& other);
    CarMatrix& operator= (CarMatrix&& other);
    ~CarMatrix();

    size_t getRows();
    size_t getCols();
    float* getValues();

    // Assumes input is a vector of length cols, and output a vector of length rows
    void multiply(const float* input, float* output);

    void mixIn(const CarMatrix& other, std::mt19937 random);
    void mutate(std::mt19937 random, float mutationChance);

    void saveToFile(std::ofstream& file);
    static CarMatrix loadFromFile(std::ifstream& file);
};

