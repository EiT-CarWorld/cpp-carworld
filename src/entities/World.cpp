#include "World.h"
#include "rendering/ModelRenderer.h"
#include "AsphaltMesh.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <random>

World::World(): m_nodes(), m_edges(), m_lineSegments(), m_routes() {}

static bool consumeNewline(std::ifstream& file) {
    if (!file.good() || file.get() != '\n') {
        std::cerr << "error: expected newline in world file" << std::endl;
        return false;
    }
    return true;
}

#define OR_RETURN(action) if(!(action)) return false
#define OR_COMPLAIN(conditional) do if(!(conditional)) {             \
std::cerr << "error: invalid condition: " #conditional << std::endl; \
return false;                                                        \
} while(false)

// Important: loading a new file will invalidate all references to the current world data
bool World::loadFromFile(const std::string& filepath) {
    std::cerr << "info: loading world from '" << filepath << "'" << std::endl;

    m_nodes.clear();
    m_edges.clear();
    m_routes.clear();
    m_lineSegments.clear();

    std::ifstream file;
    file.open(filepath);
    if (!file.good()) {
        std::cerr << "error: opening world file '" << filepath << "' failed" << std::endl;
        return false;
    }

    int num_nodes, num_edges;
    file >> num_nodes >> num_edges;
    OR_RETURN(consumeNewline(file));

    int num_lines, num_vertices, num_triangles;
    file >> num_lines >> num_vertices >> num_triangles;
    OR_RETURN(consumeNewline(file));

    // In the map positions are (x=east, y=north)
    // We change this to 3D coords (x=east,y=up,z=south)
    m_nodes.reserve(num_nodes);
    for (int i = 0; i < num_nodes; i++) {
        float x, y;
        file >> x >> y;
        OR_RETURN(consumeNewline(file));
        m_nodes.push_back(Node({x, 0, -y}));
    }

    m_edges.reserve(num_edges);
    for (int i = 0; i < num_edges; i++) {
        char direction;
        int u, v;
        file >> direction >> u >> v;
        OR_COMPLAIN(direction == 'O' || direction == 'T');
        OR_COMPLAIN(0 <= u && u < num_nodes);
        OR_COMPLAIN(0 <= v && v < num_nodes);
        OR_RETURN(consumeNewline(file));
        m_edges.push_back(Edge({&m_nodes[u], &m_nodes[v], direction=='O'}));
        m_edges[i].attach(); // Edges never move, due to reserve
    }

    // We let lines stay in (x=east,y=north) space
    m_lineSegments.reserve(num_lines);
    for (int i = 0; i < num_lines; i++) {
        float x1, y1, x2, y2;
        file >> x1 >> y1 >> x2 >> y2;
        OR_RETURN(consumeNewline(file));
        m_lineSegments.push_back(LineSegment{{x1,y1},{x2,y2}});
    }

    std::vector<float> vertices;
    vertices.reserve(num_vertices);
    for (int i = 0; i < num_vertices; i++) {
        float x, y;
        file >> x >> y;
        OR_RETURN(consumeNewline(file));
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
        OR_RETURN(consumeNewline(file));
        indices.push_back(a);
        indices.push_back(b);
        indices.push_back(c);
    }

    m_routes.push_back(Route{.nodes{&m_nodes[7], &m_nodes[8]}, .loops{false}});

    m_asphaltMesh = std::make_unique<AsphaltMesh>(&vertices[0], num_vertices, &indices[0], num_triangles*3);

    return true;
}

bool World::isLoaded() {
    return !m_nodes.empty();
}

std::vector<Route>& World::getRoutes() {
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