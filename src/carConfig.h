#pragma once

#define TARGET_FRAMERATE 60
#define TARGET_SIMULATION_FRAMERATE 60
// The delta time of each frame
#define SIM_DT (1.f/TARGET_SIMULATION_FRAMERATE)

// Number of threads used for parallel learning
#define THREAD_COUNT 2

// Updating this also requires updating the angles defined in Car.cpp
#define NUM_LIDAR_ANGLES 9
// Updating this also requires updating carZones.fs
#define NUM_CAR_ZONES 16

// Maximum distance is the distance set if nothing is closer
#define MAX_LIDAR_DIST 100.f
#define MAX_CAR_ZONE_DIST 50.0f

// Used to calculate distances between cars, taking their rectangular size in account
#define CAR_WIDTH 2.f
#define CAR_LENGTH 4.f

// Used to scale node sizes
#define ROAD_WIDTH 20.f

// =============== Score tuning ==================
// Score gained for each meter closer to next target
#define SCORE_GAIN_DISTANCE_COVER 5

// !! The following two should maybe be 0 during initial learning !!
// Score bonus for reaching the goal
#define SCORE_GOAL_REACHED_BONUS 1000
// When reaching the goal, give a bonus multiplied by its average speed (m/s) towards the goal
#define SCORE_GOAL_REACHED_SPEED_BONUS 2

// Punishment for living, in score / second
#define SCORE_TIME_PENALTY 1

// Multiply the score when crashing, larger penalty for higher speed.
// This sets the speed at which all score is removed, should be around 50 when starting learning.
#define CRASH_SPEED_MULTIPLIER_MAX 50
// !! The following should maybe be 0 during initial learning !!
// Extra penalty for crashing, multiplied with speed.
#define SCORE_CRASH_SPEED_PENALTY 0 //20

// How much score a new car spawns with
#define SCORE_INITIAL_SCORE 0
// The score at which a car gets mercy killed
#define SCORE_MINIMUM (-100)
