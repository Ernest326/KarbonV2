#pragma once
#include <entt/entt.hpp>
#include "cubemap.h"
#include "shader.h"
#include "camera.h"
#include "material_system.h"
#include "lighting_system.h"

namespace Karbon {

class Scene;

class RenderSystem {

public:
    RenderSystem(entt::registry *registry, MaterialSystem *materials, LightingSystem *lights);
    ~RenderSystem();

    // Renders the scene (skybox + PBR pass) from the given camera's view.
    void draw(Scene& scene, const Camera& camera);

    Shader& getPBRShader() { return *m_pbrShader; }

private:
    entt::registry *m_registry;
    MaterialSystem *m_materials;
    LightingSystem* m_lights;
    std::unique_ptr<Shader> m_pbrShader;
    std::unique_ptr<Shader> m_skyboxShader;
};

}
