#include "RandomRoutesPicker.h"
#include <random>

void RandomRoutesPicker::startRandomRoutePicking(int period, int minDelayBetweenSpawns, int maxDelaybetweenSpawns, int lastSpawnableFrame) {
    m_lastGenerationGenerated = -1;
    m_newPicksPeriod = period;

    m_minDelayBetweenSpawns = minDelayBetweenSpawns;
    m_maxDelayBetweenSpawns = maxDelaybetweenSpawns;
    m_lastSpawnableFrame = lastSpawnableFrame;
}

void RandomRoutesPicker::stopRandomRoutePicking() {
    m_newPicksPeriod = 0;
}

// If the spawn pattern for the active period is not the currently active spawn pattern,
// clear the spawnTimes map, and fill it with our own picks
void RandomRoutesPicker::updateRoutePicks(size_t generation, size_t seed, std::vector<Route> const& routes, std::multimap<size_t, size_t> &carSpawnTimes) {
    // Check if random spawn picking is even enabled
    if (m_newPicksPeriod == 0)
        return;

    int generationToGenerate = generation - generation % m_newPicksPeriod;
    if (generationToGenerate == m_lastGenerationGenerated)
        return;

    // Create a completely new set of car spawn times
    carSpawnTimes.clear();
    m_lastGenerationGenerated = generationToGenerate;

    // No point in adding trips, if the set of routes to chose from is empty
    if (routes.empty())
        return;

    std::mt19937 rand(seed+generationToGenerate);
    std::uniform_int_distribution<size_t> routePicker(0, routes.size() - 1);
    std::uniform_int_distribution<size_t> spawnLockTime(m_minDelayBetweenSpawns, m_maxDelayBetweenSpawns);

    std::map<Node*, size_t> spawnLocks;
    for (int i = 0; i < m_lastSpawnableFrame; i++) {
        size_t route_idx = routePicker(rand);
        Route const& route = routes[route_idx];
        Node* startNode = route.nodes.front();
        auto it = spawnLocks.find(startNode);
        if (it != spawnLocks.end()) {
            // If there exists a spawn lock on this starting node, check if its end frame has hit
            if (it->second > i)
                continue; // Skip spawning a car this frame
        }
        // It's ok to spawn a car at frame i using this route
        carSpawnTimes.insert({i, route_idx});
        size_t lockUntil = i + spawnLockTime(rand);
        spawnLocks.insert({startNode, lockUntil});
    }
}