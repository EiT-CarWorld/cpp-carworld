#include "Route.h"

bool Route::verifyRoute() {
    if (paths.size() + 1 != nodes.size())
        return false;
    for (int i = 0; i < paths.size(); i++) {
        if (paths[i]->a == nodes[i] && paths[i]->b == nodes[i + 1])
            continue;
        if (paths[i]->b == nodes[i] && paths[i]->a == nodes[i + 1] && !paths[i]->oneway)
            continue;
        return false;
    }
    if (loops && nodes.front() != nodes.back())
        return false;
    return true;
}