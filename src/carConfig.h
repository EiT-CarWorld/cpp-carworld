#pragma once

#define TARGET_FRAMERATE 60
#define TARGET_SIMULATION_FRAMERATE 60
// The delta time of each frame
#define SIM_DT (1.f/TARGET_SIMULATION_FRAMERATE)

// Updating this also requires updating the angles defined in Car.cpp
#define NUM_LIDAR_ANGLES 5
// Updating this also requires updating carZones.fs
#define NUM_CAR_ZONES 16

// Used to calculate distances between cars, taking their rectangular size in account
#define CAR_WIDTH 2.f
#define CAR_LENGTH 4.f

// Used to scale roads and node sizes
#define ROAD_WIDTH 12.f

#define SCORE_CRASH_PENALTY 1000
#define SCORE_GAIN_DISTANCE_COVER 1
#define SCORE_TIME_PENALTY 10
#define SCORE_INITIAL_SCORE 100