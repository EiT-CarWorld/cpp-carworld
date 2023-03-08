#pragma once

#include <vector>
#include <cstddef>
#include "entities/Node.h"
#include "entities/Edge.h"

struct Route {
    std::vector<Node*> nodes{}; // Size equal to paths
    bool loops{};
};

