#include <cassert>
#include "Path.h"
#include "Node.h"
#include "rendering/ModelRenderer.h"
#include "carMath.h"

Texture Path::asphalt, Path::asphaltSpecular;
Model Path::roadModel, Path::onewayRoadModel, Path::pathNodeModel;

void Path::loadStatic() {
    asphalt = LoadTexture("res/textures/asphalt.png");
    GenTextureMipmaps(&asphalt);
    SetTextureFilter(asphalt, TEXTURE_FILTER_BILINEAR);
    asphaltSpecular = LoadTexture("res/textures/asphalt_specular.png");
    GenTextureMipmaps(&asphaltSpecular);
    SetTextureFilter(asphaltSpecular, TEXTURE_FILTER_BILINEAR);

    roadModel = ModelRenderer::loadModel("res/models/road.glb");
    onewayRoadModel = ModelRenderer::loadModel("res/models/roadOneWay.glb");
    pathNodeModel = ModelRenderer::loadModel("res/models/pathnode.glb");

    roadModel.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = asphalt;
    roadModel.materials[1].maps[MATERIAL_MAP_SPECULAR].texture = asphaltSpecular;
    roadModel.materials[1].maps[MATERIAL_MAP_SPECULAR].color = {80, 80, 80};
    onewayRoadModel.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = asphalt;
    onewayRoadModel.materials[1].maps[MATERIAL_MAP_SPECULAR].texture = asphaltSpecular;
    onewayRoadModel.materials[1].maps[MATERIAL_MAP_SPECULAR].color = {80, 80, 80};
    pathNodeModel.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = asphalt;
    pathNodeModel.materials[1].maps[MATERIAL_MAP_SPECULAR].texture = asphaltSpecular;
    pathNodeModel.materials[1].maps[MATERIAL_MAP_SPECULAR].color = {80, 80, 80};
}

void Path::unloadStatic() {
    ModelRenderer::unloadModel(roadModel);
    ModelRenderer::unloadModel(onewayRoadModel);
    ModelRenderer::unloadModel(pathNodeModel);
    UnloadTexture(asphalt);
    UnloadTexture(asphaltSpecular);
}

Path::Path(Node *a, Node *b, bool oneway, PathNode *path_nodes, size_t path_node_count) :
        a(a), b(b), oneway(oneway), path_nodes(path_nodes), path_node_count(path_node_count) {
    a->addedToPath(this);
    b->addedToPath(this);
}

void Path::render() {
    Model &model = oneway ? onewayRoadModel : roadModel;

    assert (a != nullptr && b != nullptr);
    Vector3 prevPos = a->position;
    for (int i = 0; i <= path_node_count; i++) {
        PathNode* to;
        if(i < path_node_count)
            to = &path_nodes[i];
        else
            to = b;

        // Draw road from prevPos to nextNode
        Vector3 diff = to->position - prevPos;
        float length = Vector3Length(diff);
        DrawModelEx(model, prevPos,
                    {0, 1, 0}, atan2(-diff.z, diff.x) * RAD2DEG,
                    {length, 1, ROAD_WIDTH}, WHITE);
        prevPos = to->position;

        if(i < path_node_count)
            to->render();
    }
}

void PathNode::render() {
    DrawModelEx(Path::pathNodeModel, position,
                {0,1,0}, 0, {diameter, 1, diameter}, WHITE);
}