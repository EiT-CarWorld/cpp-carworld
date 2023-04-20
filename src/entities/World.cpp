#include "World.h"
#include "rendering/ModelRenderer.h"
#include "AsphaltMesh.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <random>
#include <cassert>
#include <queue>
#include <unordered_map>

World::World(): m_nodes(), m_lineSegments(), m_routes() {}

static bool consumeNewline(std::ifstream& file) {
    if (file.get() == '\n')
        return true;
    std::cerr << "error: expected newline in world file" << std::endl;
    return false;
}

#define OR_RETURN(action) if(!(action)) return false
#define OR_COMPLAIN(conditional) do if(!(conditional)) {             \
std::cerr << "error: invalid condition: " #conditional << std::endl; \
return false;                                                        \
} while(false)

// Important: loading a new file will invalidate all references to the current world data
bool World::loadFromFile(const std::string& filepath) {
    TraceLog(LOG_INFO, "loading world file from %s", filepath.c_str());

    m_nodes.clear();
    m_routes.clear();
    m_lineSegments.clear();

    std::ifstream file;
    file.open(filepath);
    OR_COMPLAIN(file.good());

    int num_nodes, num_edges;
    file >> num_nodes >> num_edges;
    OR_COMPLAIN(consumeNewline(file));

    int num_lines, num_vertices, num_triangles;
    file >> num_lines >> num_vertices >> num_triangles;
    OR_COMPLAIN(consumeNewline(file));

    // In the map positions are (x=east, y=north)
    // We change this to 3D coords (x=east,y=up,z=south)
    m_nodes.reserve(num_nodes);
    for (int i = 0; i < num_nodes; i++) {
        float x, y;
        file >> x >> y;
        OR_COMPLAIN(consumeNewline(file));
        m_nodes.push_back(Node({x, 0, -y}));
    }

    for (int i = 0; i < num_edges; i++) {
        char direction;
        int u, v;
        file >> direction >> u >> v;
        OR_COMPLAIN(direction == 'O' || direction == 'T');
        OR_COMPLAIN(0 <= u && u < num_nodes);
        OR_COMPLAIN(0 <= v && v < num_nodes);
        OR_COMPLAIN(consumeNewline(file));
        m_nodes[u].addNeighbour(&m_nodes[v]);
        if (direction == 'T')
            m_nodes[v].addNeighbour(&m_nodes[u]);
    }

    // We let lines stay in (x=east,y=north) space
    m_lineSegments.reserve(num_lines);
    for (int i = 0; i < num_lines; i++) {
        float x1, y1, x2, y2;
        file >> x1 >> y1 >> x2 >> y2;
        OR_COMPLAIN(consumeNewline(file));
        m_lineSegments.push_back(LineSegment{{x1,y1},{x2,y2}});
    }

    std::vector<float> vertices;
    vertices.reserve(num_vertices);
    for (int i = 0; i < num_vertices; i++) {
        float x, y;
        file >> x >> y;
        OR_COMPLAIN(consumeNewline(file));
        // Convert from (x=east,y=north) to (x=east,y=up,z=south)
        vertices.push_back(x);
        vertices.push_back(0.f);
        vertices.push_back(-y);
    }

    std::vector<unsigned int> indices;
    vertices.reserve(num_vertices);
    for (int i = 0; i < num_triangles; i++) {
        unsigned int a, b, c;
        file >> a >> b >> c;
        OR_COMPLAIN(consumeNewline(file));
        indices.push_back(a);
        indices.push_back(b);
        indices.push_back(c);
    }

    m_asphaltMesh = std::make_unique<AsphaltMesh>(&vertices[0], num_vertices, &indices[0], num_triangles*3);

    return true;
}

void World::clearRoutes() {
    m_routes.clear();
}

void World::addRoute(size_t from, size_t to) {
    assert(from >= 0 && from <= m_nodes.size());
    assert(to >= 0 && to <= m_nodes.size());

    // -distance, {node, previous}
    std::priority_queue<std::pair<float,std::pair<Node*, Node*>>> qu;
    qu.push({0.0f, {&m_nodes[from], nullptr}});
    // from node to the shortest prev to that node
    std::unordered_map<Node*, Node*> backtrack;

    while (true) {
        if (qu.empty()) __builtin_trap();

        float dist = -qu.top().first;
        Node* node = qu.top().second.first;
        Node* prev = qu.top().second.second;
        qu.pop();

        if (backtrack.count(node))
            continue;
        backtrack.insert({node, prev});

        if (node == &m_nodes[to])
            break;

        for (Node* neighbour:node->neighbours) {
            if (backtrack.count(neighbour))
                continue;
            float extraDist = Vector3Length(neighbour->position - node->position);
            float totalDist = dist + extraDist;
            qu.push({-totalDist, {neighbour, node}});
        }
    }

    std::vector<Node*> route_reverse;
    Node* node = &m_nodes[to];
    while(node != nullptr) {
        route_reverse.push_back(node);
        node = backtrack.find(node)->second;
    }
    Route route{std::vector(route_reverse.rbegin(), route_reverse.rend()), false};
    m_routes.emplace_back(std::move(route));
}

bool World::isLoaded() {
    return !m_nodes.empty();
}

const std::vector<Node>& World::getNodes() {
    return m_nodes;
}

const std::vector<Route>& World::getRoutes() {
    return m_routes;
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

void World::render() {
    ModelRenderer::setMode(GLOBAL_TEXTURE_MODE);
    m_asphaltMesh->render();
    ModelRenderer::setMode(MODEL_MODE);
}

void World::renderRoadBorders() {
    for(auto& lineSegment : m_lineSegments)
        lineSegment.render();
}