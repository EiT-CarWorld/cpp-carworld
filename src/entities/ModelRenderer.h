#include "raylib.h"

class ModelRenderer {
private:
    static Shader shader;
    static int ambientLightLoc, directionalLightLoc, directionalLightDirectionLoc;

    Vector3 m_ambientLight, m_directionalLight, m_directionalLightDirection;
public:
    static void loadStatic();
    static void unloadStatic();
    static Model loadModel(char const *name);
    static void unloadModel(Model model);

    ModelRenderer(Vector3 ambientLight, Vector3 directionalLight, Vector3 directionalLightDirection);
    ~ModelRenderer();
    void uploadState(Camera3D camera);
};
