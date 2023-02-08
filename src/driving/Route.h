#pragma once

#include <vector>
#include <cstddef>
#include "entities/Node.h"
#include "entities/Path.h"

struct Route {
    // A Path connects two nodes, with 0 or more PathNodes in between
    std::vector<Path*> paths;
    std::vector<Node*> nodes; // Size equal to paths+1
    bool loops;

    bool verifyRoute();
};

