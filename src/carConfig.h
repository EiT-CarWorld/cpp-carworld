#pragma once

#define TARGET_FRAMERATE 60
#define TARGET_SIMULATION_FRAMERATE 60
// The delta time of each frame
#define SIM_DT (1.f/TARGET_SIMULATION_FRAMERATE)

// Number of threads used for parallel learning
#define THREAD_COUNT 8

// Updating this also requires updating the angles defined in Car.cpp
#define NUM_LIDAR_ANGLES 9
// Updating this also requires updating carZones.fs
#define NUM_CAR_ZONES 16

// Used to calculate distances between cars, taking their rectangular size in account
#define CAR_WIDTH 2.f
#define CAR_LENGTH 4.f

// Used to scale roads and node sizes
#define ROAD_WIDTH 14.f

// Score tuning
#define SCORE_CRASH_PENALTY 100
// Extra penalty for crashing, multiplied with speed
#define SCORE_CRASH_SPEED_PENALTY 0
#define SCORE_GAIN_DISTANCE_COVER 1
#define SCORE_TIME_PENALTY 4
#define SCORE_INITIAL_SCORE 0
// The score at which it dies
#define SCORE_MINIMUM -100
