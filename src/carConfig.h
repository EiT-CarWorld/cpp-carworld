#pragma once

#define TARGET_FRAMERATE 60
#define TARGET_SIMULATION_FRAMERATE 60
// The delta time of each frame
#define SIM_DT (1.f/TARGET_SIMULATION_FRAMERATE)

// Updating this also requires updating the angles defined in Car.cpp
#define NUM_LIDAR_ANGLES 5
// Updating this also requires updating carZones.fs
#define NUM_CAR_ZONES 16

#define ROAD_WIDTH 12.f