#include <cassert>
#include "Node.h"
#include "ModelRenderer.h"
#include "carMath.h"

Texture Node::asphalt;
Model Node::roadModel, Node::pathNodeModel;

void Node::loadStatic() {
    asphalt = LoadTexture("res/textures/asphalt.png");
    GenTextureMipmaps(&asphalt);
    SetTextureFilter(asphalt, TEXTURE_FILTER_BILINEAR);

    roadModel = ModelRenderer::loadModel("res/models/road.glb");
    pathNodeModel = ModelRenderer::loadModel("res/models/pathnode.glb");

    roadModel.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = asphalt;
    pathNodeModel.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = asphalt;
}

void Node::unloadStatic() {
    ModelRenderer::unloadModel(roadModel);
    UnloadTexture(asphalt);
}

void Node::addOutPath(Path *path) {
    assert(path->from == nullptr);
    path->from = this;
    int index = (m_outpaths[0] != nullptr) + (m_outpaths[1] != nullptr);
    assert(index < 2 && "Mininode already had two output paths");
    m_outpaths[index] = path;
}

void Node::addInPath(Path *path) {
    assert(path->to == nullptr);
    path->to = this;
    int index = (m_inpaths[0] != nullptr) + (m_inpaths[1] != nullptr);
    assert(index < 2 && "Mininode already had two input paths");
    m_inpaths[index] = path;
}

void Node::render() {
    DrawModelEx(pathNodeModel, m_position,
                {0,1,0}, 0, {ROAD_WIDTH+.5, 1, ROAD_WIDTH+.5}, WHITE);

    for (auto & m_outpath : m_outpaths) {
        if (m_outpath == nullptr || m_outpath->to == nullptr) continue;
        Vector3 prevPos = m_position;
        for (int i = 0; ; i++) {
            Vector3 to;
            if(i < m_outpath->pathNodeCount)
                to = m_outpath->pathNodes[i].position;
            else
                to = m_outpath->to->m_position;

            // Draw road from prevPos to nextNode
            Vector3 diff = to - prevPos;
            float length = Vector3Length(diff);
            DrawModelEx(roadModel, prevPos,
                        {0, 1, 0}, atan2(-diff.z, diff.x) * RAD2DEG,
                        {length, 1, ROAD_WIDTH}, WHITE);
            prevPos = to;

            if(i < m_outpath->pathNodeCount)
                DrawModelEx(pathNodeModel, to,
                            {0,1,0}, 0, {ROAD_WIDTH, 1, ROAD_WIDTH}, WHITE);
            else
                break;
        }
    }
}