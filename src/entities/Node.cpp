#include <cassert>
#include "Node.h"
#include "carMath.h"

void Node::addedToPath(Path *path) {
    m_paths.push_back(path);
}

void Node::render() {
    DrawModelEx(Path::pathNodeModel, m_position,
                {0,1,0}, 0, {ROAD_WIDTH+1, 1.1, ROAD_WIDTH+1}, WHITE);
}