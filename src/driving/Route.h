#pragma once

#include <vector>
#include <cstddef>
#include "entities/Path.h"

struct Route {
    std::vector<Path*> paths;
    bool loops;

    bool verifyRoute();
};

