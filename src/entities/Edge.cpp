#include <cassert>
#include "Edge.h"
#include "Node.h"
#include "rendering/ModelRenderer.h"
#include "carMath.h"

Texture Edge::asphalt, Edge::asphaltSpecular;
Model Edge::roadModel, Edge::onewayRoadModel, Edge::nodeModel;

void Edge::loadStatic() {
    asphalt = LoadTexture("res/textures/asphalt.png");
    GenTextureMipmaps(&asphalt);
    SetTextureFilter(asphalt, TEXTURE_FILTER_BILINEAR);
    asphaltSpecular = LoadTexture("res/textures/asphalt_specular.png");
    GenTextureMipmaps(&asphaltSpecular);
    SetTextureFilter(asphaltSpecular, TEXTURE_FILTER_BILINEAR);

    roadModel = ModelRenderer::loadModel("res/models/road.glb");
    onewayRoadModel = ModelRenderer::loadModel("res/models/roadOneWay.glb");
    nodeModel = ModelRenderer::loadModel("res/models/pathnode.glb");

    roadModel.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = asphalt;
    roadModel.materials[1].maps[MATERIAL_MAP_SPECULAR].texture = asphaltSpecular;
    roadModel.materials[1].maps[MATERIAL_MAP_SPECULAR].color = {80, 80, 80};
    onewayRoadModel.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = asphalt;
    onewayRoadModel.materials[1].maps[MATERIAL_MAP_SPECULAR].texture = asphaltSpecular;
    onewayRoadModel.materials[1].maps[MATERIAL_MAP_SPECULAR].color = {80, 80, 80};
    nodeModel.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = asphalt;
    nodeModel.materials[1].maps[MATERIAL_MAP_SPECULAR].texture = asphaltSpecular;
    nodeModel.materials[1].maps[MATERIAL_MAP_SPECULAR].color = {80, 80, 80};
}

void Edge::unloadStatic() {
    ModelRenderer::unloadModel(roadModel);
    ModelRenderer::unloadModel(onewayRoadModel);
    ModelRenderer::unloadModel(nodeModel);
    UnloadTexture(asphalt);
    UnloadTexture(asphaltSpecular);
}

Edge::Edge(Node* a, Node* b, bool oneway) : a(a), b(b), oneway(oneway) {}

void Edge::attach() {
    a->addedToEdge(this);
    b->addedToEdge(this);
}

void Edge::render() {
    Model &model = oneway ? onewayRoadModel : roadModel;

    assert (a != nullptr && b != nullptr);
    Vector3 diff = b->position - a->position;
    float length = Vector3Length(diff);
    DrawModelEx(model, a->position,
                {0, 1, 0}, atan2(-diff.z, diff.x) * RAD2DEG,
                {length, 1, ROAD_WIDTH}, WHITE);
}