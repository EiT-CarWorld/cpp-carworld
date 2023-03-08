#include "AsphaltMesh.h"
#include "rendering/ModelRenderer.h"
#include "rlgl.h"
#include "raymath.h"
#include "external/glad.h"
#include <iostream>

Texture AsphaltMesh::asphaltTexture, AsphaltMesh::specularTexture;

void AsphaltMesh::loadStatic() {
    asphaltTexture = LoadTexture("res/textures/asphalt.png");
    GenTextureMipmaps(&asphaltTexture);
    SetTextureFilter(asphaltTexture, TEXTURE_FILTER_BILINEAR);
    specularTexture = LoadTexture("res/textures/asphalt_specular.png");
    GenTextureMipmaps(&specularTexture);
    SetTextureFilter(specularTexture, TEXTURE_FILTER_BILINEAR);
}

void AsphaltMesh::unloadStatic() {
    UnloadTexture(asphaltTexture);
    UnloadTexture(specularTexture);
}

// Once the mesh is uploaded, the data passed here is no longer needed
AsphaltMesh::AsphaltMesh(float* vertices, int num_vertices, unsigned int* indices, int num_indices)
        : num_vertices(num_vertices), num_indices(num_indices) {
    vaoId = rlLoadVertexArray();

    if (vaoId == 0) {
        std::cerr << "VertexAttributeArrays are not supported" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    rlEnableVertexArray(vaoId); // Enable writing Vertex Buffers to the VAO

    positionVboId = rlLoadVertexBuffer(vertices, num_vertices*3*sizeof(float), false);
    rlSetVertexAttribute(0, 3, RL_FLOAT, 0, 0, 0);
    rlEnableVertexAttribute(0);

    rlDisableVertexAttribute(1); // No texture coordinate

    // Give all vertices a normal going straight up
    float value[3] = { 0.0f, 1.0f, 0.0f };
    rlSetVertexAttributeDefault(2, value, RL_SHADER_ATTRIB_VEC3, 3);
    rlDisableVertexAttribute(2);

    indicesVboId = rlLoadVertexBufferElement(indices, num_indices*sizeof(unsigned int), false);

    rlDisableVertexArray();
}

AsphaltMesh::~AsphaltMesh() {
    rlUnloadVertexBuffer(positionVboId);
    rlUnloadVertexArray(indicesVboId);
    rlUnloadVertexArray(vaoId);
}

void AsphaltMesh::render() {
    rlEnableVertexArray(vaoId);
    ModelRenderer::uploadMatrices(MatrixIdentity());
    ModelRenderer::uploadMaterial({1.f, 1.f, 1.f, 1.f}, asphaltTexture.id, {.3f, .3f, .3f, 1.f}, specularTexture.id);
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, nullptr);
    rlDisableVertexArray();
}