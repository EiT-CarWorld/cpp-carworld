#include "World.h"
#include "rendering/ModelRenderer.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <random>

World::World(): m_nodes(), m_pathNodes(), m_paths() {}

static void assertNewline(std::ifstream& in) {
    assert(in.get() == '\n');
    assert(in.good());
}

void World::loadFromFile(const std::string& path) {
    std::ifstream file;
    file.open(path);
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
        Path* last_path = nullptr;
        Node* head_node = start_node;
        while (true) {
            if (head_node->m_paths.size() <= 1) // We can't go on from here
                break;
            std::uniform_int_distribution<size_t> path_choice(0, head_node->m_paths.size()-1);
            Path* next_path = head_node->m_paths[path_choice(gen)];
            if (next_path == last_path)
                continue; // Try again until we pick a path that isn't last_path

            route.paths.push_back(next_path);
            last_path = next_path;
            head_node = next_path->m_a == head_node ? next_path->m_b : next_path->m_a;
            if (head_node == start_node) {
                route.loops = true;
                break;
            }
        }

        if (!route.paths.empty())
            m_routes.emplace_back(std::move(route));
    }
}

void World::spawnCar() {
    assert(!m_routes.empty());

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> route_choice(0, m_routes.size()-1);
    m_cars.emplace_back(std::make_unique<Car>(&m_routes[route_choice(gen)]));
}

size_t World::getCarCount() {
    return m_cars.size();
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
}