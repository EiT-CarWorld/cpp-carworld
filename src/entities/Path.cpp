#include <cassert>
#include "Path.h"
#include "Node.h"
#include "ModelRenderer.h"
#include "carMath.h"

Texture Path::asphalt;
Model Path::roadModel, Path::onewayRoadModel, Path::pathNodeModel;

void Path::loadStatic() {
    asphalt = LoadTexture("res/textures/asphalt.png");
    GenTextureMipmaps(&asphalt);
    SetTextureFilter(asphalt, TEXTURE_FILTER_BILINEAR);

    roadModel = ModelRenderer::loadModel("res/models/road.glb");
    onewayRoadModel = ModelRenderer::loadModel("res/models/roadOneWay.glb");
    pathNodeModel = ModelRenderer::loadModel("res/models/pathnode.glb");

    roadModel.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = asphalt;
    onewayRoadModel.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = asphalt;
    pathNodeModel.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = asphalt;
}

void Path::unloadStatic() {
    ModelRenderer::unloadModel(roadModel);
    ModelRenderer::unloadModel(onewayRoadModel);
    UnloadTexture(asphalt);
}

Path::Path(Node *a, Node *b, bool oneway, PathNode *nodes, int nodeCount) :
        m_a(a), m_b(b), m_oneway(oneway), m_pathNodes(nodes), m_pathNodeCount(nodeCount) {
    m_a->addedToPath(this);
    m_b->addedToPath(this);
}

void Path::render() {
    Model &model = m_oneway ? onewayRoadModel : roadModel;

    assert (m_a != nullptr && m_b != nullptr);
    Vector3 prevPos = m_a->m_position;
    for (int i = 0; i <= m_pathNodeCount; i++) {
        Vector3 to;
        if(i < m_pathNodeCount)
            to = m_pathNodes[i].position;
        else
            to = m_b->m_position;

        // Draw road from prevPos to nextNode
        Vector3 diff = to - prevPos;
        float length = Vector3Length(diff);
        DrawModelEx(model, prevPos,
                    {0, 1, 0}, atan2(-diff.z, diff.x) * RAD2DEG,
                    {length, 1, ROAD_WIDTH}, WHITE);
        prevPos = to;

        if(i < m_pathNodeCount)
            DrawModelEx(pathNodeModel, to,
                        {0,1,0}, 0, {ROAD_WIDTH, 1, ROAD_WIDTH}, WHITE);
    }
}