#pragma once
#include <stddef.h>
#include <map>
#include "Route.h"

class RandomRoutesPicker {
    // The last time the picker generated new routes
    // This is done to allow determinism when restoring from file
    int m_lastGenerationGenerated{0};

    // How long each period lasts before new spawn times are picked
    int m_newPicksPeriod{0};

    // How long between each frame where we attempt to spawn a car
    int m_spawnableFramePeriod{};
    // When picking spawn times, how long should it at least be between two cars spawn at the same node
    int m_minDelayBetweenSpawns{};
    // Not an actual max, but the restriction placed on a start node can at most last for this long
    int m_maxDelayBetweenSpawns{};
    // Only generate cars until the given frame
    int m_lastSpawnableFrame{};

public:
    void startRandomRoutePicking(int period, int spawnableFramePeriod, int minDelayBetweenSpawns, int maxDelayBetweenSpawns, int lastSpawnableFrame);
    void stopRandomRoutePicking();

    // The seed is combined with generation, so can stay constant
    void updateRoutePicks(size_t generation, size_t seed, std::vector<Route> const& routes, std::multimap<size_t, size_t>& carSpawnTimes);
};
