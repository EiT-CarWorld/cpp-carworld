#include <cassert>
#include "Route.h"

bool Route::verifyRoute() {
    assert(paths.size() + 1 == nodes.size());
    for(int i = 0; i < paths.size(); i++) {
        if (paths[i]->a == nodes[i] && paths[i]->b == nodes[i + 1])
            continue;
        if (paths[i]->b == nodes[i] && paths[i]->a == nodes[i + 1] && !paths[i]->oneway)
            continue;
        return false;
    }
    return true;
}