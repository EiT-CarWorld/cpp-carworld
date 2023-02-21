#include "CarBrain.h"

CarBrainOutput CarBrain::takeAction(CarBrainInput input) {
    return {GAS_DRIVE, TURN_NO_TURN};
}