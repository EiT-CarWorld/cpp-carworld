#include "World.h"
#include "ModelRenderer.h"
#include <iostream>
#include <fstream>
#include <cassert>

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

        m_paths.push_back(Path({&m_nodes[F], &m_nodes[T], directionality=='O', &m_pathNodes[used_path_nodes], V}));
        used_path_nodes += V;
    }

    assert(used_path_nodes == num_path_nodes && "The input didn't use the exact amount of path nodes given");

    assert(file.peek() == EOF && "File had more content after the end of the data");
}

void World::render() {
    ModelRenderer::setMode(GLOBAL_TEXTURE_MODE);
    for(Node& node : m_nodes)
        node.render();
    for(Path& path: m_paths)
        path.render();
    ModelRenderer::setMode(MODEL_MODE);
}