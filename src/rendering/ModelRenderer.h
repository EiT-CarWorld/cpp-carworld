#include "raylib.h"

enum ModelShaderMode {
    MODEL_MODE = 0,
    GLOBAL_TEXTURE_MODE = 1,
};

class ModelRenderer {
private:
    static Shader shader;
    static int ambientLightLoc, directionalLightLoc, directionalLightDirectionLoc;
    static int shaderModeLoc;

    Vector3 m_ambientLight, m_directionalLight, m_directionalLightDirection;
public:
    static void loadStatic();
    static void unloadStatic();
    static Model loadModel(char const *name);
    static void unloadModel(Model model);
    static void setMode(ModelShaderMode mode);
    static void uploadMatrices(Matrix transform);
    static void uploadMaterial(Vector4 diffuse, unsigned int diffuseTexture, Vector4 specular, unsigned int specularTexture);

    ModelRenderer(Vector3 ambientLight, Vector3 directionalLight, Vector3 directionalLightDirection);
    ~ModelRenderer();
    void uploadState(Camera3D camera);
};
