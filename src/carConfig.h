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

// =============== Score tuning ==================
// Penalty for crashing
#define SCORE_CRASH_PENALTY 100
// Extra penalty for crashing, multiplied with speed
#define SCORE_CRASH_SPEED_PENALTY 1
// Penalty for crashing by driving of the road
#define SCORE_CRASH_ROADSIDE_PENALTY 300
// Penalty for crashing with another car, being the faster of the two
#define SCORE_CRASH_FASTEST_PENALTY 300
// Penalty for crashing with another car, and being the slower of the two
#define SCORE_CRASH_SLOWEST_PENALTY 0

// Punishment for living, in score / second
#define SCORE_TIME_PENALTY 4
// Score gained for each meter closer to next target
#define SCORE_GAIN_DISTANCE_COVER 1

// How much score a new car spawns with
#define SCORE_INITIAL_SCORE 400
// The score at which a car gets mercy killed
#define SCORE_MINIMUM 300
