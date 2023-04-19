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

// Used to scale node sizes
#define ROAD_WIDTH 14.f

// =============== Score tuning ==================
// Penalty for crashing
//#define SCORE_CRASH_PENALTY 500
// Extra penalty for crashing, multiplied with speed
//#define SCORE_CRASH_SPEED_PENALTY 20
// Penalty for crashing by driving off the road
//#define SCORE_CRASH_ROADSIDE_PENALTY (-100)
// Penalty for crashing with another car, being the faster of the two
//#define SCORE_CRASH_FASTEST_PENALTY 0
// Penalty for crashing with another car, and being the slower of the two
//#define SCORE_CRASH_SLOWEST_PENALTY 0

// Score gained for each meter closer to next target
#define SCORE_GAIN_DISTANCE_COVER 5

// Score bonus for reaching the goal
#define SCORE_GOAL_REACHED_BONUS 1000
// When reaching the goal, give a bonus multiplied by its average speed towards the goal
// In score * seconds/meter
#define SCORE_GOAL_REACHED_SPEED_BONUS 5
// Punishment for living, in score / second
#define SCORE_TIME_PENALTY 1

// Multiply the score when crashing, larger penalty for higher speed
#define CRASH_SPEED_MULTIPLIER_MAX 50

// How much score a new car spawns with
#define SCORE_INITIAL_SCORE 0
// The score at which a car gets mercy killed
#define SCORE_MINIMUM (-100)
