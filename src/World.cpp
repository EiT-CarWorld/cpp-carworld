#include "World.h"
#include "rendering/ModelRenderer.h"
#include "rlgl.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <random>

World::World(): m_nodes(), m_pathNodes(), m_paths(), m_lineSegments(), m_routes(), m_cars() {}

static void assertNewline(std::ifstream& in) {
    assert(in.get() == '\n');
    assert(in.good());
}

void World::loadFromFile(const std::string& filepath) {
    std::ifstream file;
    file.open(filepath);
    assert(file.is_open());
    int num_nodes, num_path_nodes, num_paths;
    file >> num_nodes >> num_path_nodes >> num_paths;
    assertNewline(file);

    m_nodes.clear();
    m_pathNodes.clear();
    m_paths.clear();
    m_nodes.reserve(num_nodes);
    m_pathNodes.reserve(num_path_nodes);
    m_paths.reserve(num_paths);

    for (int i = 0; i < num_nodes; i++) {
        float x, z;
        file >> x >> z;
        assertNewline(file);
        m_nodes.push_back(Node({x, 0, z}));
    }

    for (int i = 0; i < num_path_nodes; i++) {
        float x, z;
        file >> x >> z;
        assertNewline(file);
        m_pathNodes.push_back(PathNode({x, 0, z}));
    }

    int used_path_nodes = 0;
    for (int i = 0; i < num_paths; i++) {
        char directionality;
        int F, V, T;
        file >> directionality >> F >> V >> T;
        assertNewline(file);
        // Must be either one-way or two-way
        assert(directionality == 'O' || directionality == 'T');
        assert(0 <= F && F < num_nodes);
        assert(0 <= T && T < num_nodes);
        assert(0 <= V);

        m_paths.emplace_back(std::make_unique<Path>(&m_nodes[F], &m_nodes[T], directionality=='O',
                                                    &m_pathNodes[used_path_nodes], (size_t) V));
        used_path_nodes += V;
    }

    assert(used_path_nodes == num_path_nodes && "The input didn't use the exact amount of path nodes given");
    assert(file.peek() == EOF && "File had more content after the end of the data");

    // Now populate line segment set
    const float radius = (ROAD_WIDTH / 2);
    for (auto& path : m_paths) {
        Vector2 lastPos{path->a->position.x, path->a->position.z}, nextPos;
        Vector2 lastLeftCorner, lastRightCorner;
        for (int i = 0; i < path->path_node_count; i++) {
            Vector2 pos {path->path_nodes[i].position.x, path->path_nodes[i].position.z};
            if (i == 0) { // lastPos is moved closer, to leave the node open
                Vector2 forwards = Vector2Normalize(pos - lastPos);
                lastPos += radius * forwards;
                lastLeftCorner = lastPos + radius * Vector2{-forwards.y, forwards.x};
                lastRightCorner = lastPos + radius * Vector2{forwards.y, -forwards.x};
            }

            if (i+1 < path->path_node_count) {
                nextPos = { path->path_nodes[i+1].position.x, path->path_nodes[i+1].position.z };
            } else {
                nextPos = {path->b->position.x, path->b->position.z};
                nextPos -= radius * Vector2Normalize(nextPos - pos);
            }

            Vector2 angleIn = Vector2Normalize(pos - lastPos);
            Vector2 angleOut = Vector2Normalize(nextPos - pos);
            Vector2 tangent = Vector2Normalize(angleIn + angleOut);
            float our_radius = radius * (1 + (1 - Vector2DotProduct(angleIn, angleOut))/2 );
            Vector2 leftCorner = pos + our_radius * Vector2{-tangent.y, tangent.x};
            Vector2 rightCorner = pos + our_radius * Vector2{tangent.y, -tangent.x};

            m_lineSegments.push_back(LineSegment{lastLeftCorner, leftCorner});
            m_lineSegments.push_back(LineSegment{lastRightCorner, rightCorner});

            lastLeftCorner = leftCorner;
            lastRightCorner = rightCorner;
            lastPos = pos;
        }
        // finally add the lines to the intersection node ending our path
        Vector2 forwards = Vector2Normalize(nextPos - lastPos);
        Vector2 leftCorner = nextPos + radius * Vector2{-forwards.y, forwards.x};
        Vector2 rightCorner = nextPos + radius * Vector2{forwards.y, -forwards.x};
        m_lineSegments.push_back(LineSegment{lastLeftCorner, leftCorner});
        m_lineSegments.push_back(LineSegment{lastRightCorner, rightCorner});
    }
}

// TODO: Do A* and stuff to find paths, not just random
void World::createRoutes(unsigned seed, size_t count) {
    // Since cars have pointers into the m_routes vector,
    // we may not add or remove routes while ANY car is alive
    assert(m_cars.empty());
    assert(!m_nodes.empty());

    // Remove any old routes
    m_routes.clear();
    m_routes.reserve(count);

    std::mt19937 gen(seed);
    std::uniform_int_distribution<size_t> node_choice(0, m_nodes.size()-1);

    // For now, we only create loops
    while(m_routes.size() < count) {
        Node* start_node = &m_nodes[node_choice(gen)];
        Route route;
        route.nodes.push_back(start_node);

        Path* last_path = nullptr;
        Node* head_node = start_node;
        while (true) {
            if (head_node->paths.size() <= 1) // We can't go on from here
                break;
            std::uniform_int_distribution<size_t> path_choice(0, head_node->paths.size()-1);
            Path* next_path = head_node->paths[path_choice(gen)];
            if (next_path == last_path)
                continue; // Try again until we pick a path that isn't last_path

            route.paths.push_back(next_path);
            last_path = next_path;
            head_node = next_path->a == head_node ? next_path->b : next_path->a;
            route.nodes.push_back(head_node);

            if (head_node == start_node) {
                route.loops = true;
                break;
            }
        }

        assert(route.verifyRoute());

        if (!route.paths.empty())
            m_routes.emplace_back(std::move(route));
    }
}

float World::getRayDistance(Vector2 pos, Vector2 dir, float max_distance) {
    float distance = max_distance;
    for (auto& line : m_lineSegments) {
        std::optional<float> dist = line.getRayDistance(pos, dir);
        if (dist.has_value())
            distance = fminf(distance, dist.value());
    }
    return distance;
}

void World::spawnCar() {
    assert(!m_routes.empty());

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> route_choice(0, m_routes.size()-1);
    m_cars.emplace_back(std::make_unique<Car>(&m_routes[route_choice(gen)]));
}

std::vector<std::unique_ptr<Car>>& World::getCars() {
    return m_cars;
}

void World::takeCarActions() {
    for(auto& car:m_cars)
        car->chooseAction(this);
}

void World::updateCars() {
    for (int i = 0; i < m_cars.size(); i++) {
        m_cars[i]->update(this);
        if(m_cars[i]->hasFinishedRoute()) {
            m_cars[i].swap(m_cars.back());
            m_cars.pop_back();
            i--; // To re-do index i
        }
    }
}

void World::render() {
    ModelRenderer::setMode(GLOBAL_TEXTURE_MODE);
    for(Node& node : m_nodes)
        node.render();
    for(auto& path: m_paths)
        path->render();
    ModelRenderer::setMode(MODEL_MODE);
    for(auto& car: m_cars)
        car->render();
    for(auto& lineSegment : m_lineSegments)
        lineSegment.render();

    rlDisableDepthMask();
    for(auto& car : m_cars)
        car->renderSensory();
    rlEnableDepthMask();
}