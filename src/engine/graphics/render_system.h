#pragma once
#include <entt/entt.hpp>
#include "cubemap.h"
#include "shader.h"
#include "material_system.h"
#include "lighting_system.h"

namespace Karbon {

class RenderSystem {

public:
    RenderSystem(entt::registry *registry, MaterialSystem *materials, LightingSystem *lights);
    ~RenderSystem();
    void Draw(Shader* shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos);
    void setSkybox(Cubemap* skybox) { m_skybox = skybox; }

private:
    entt::registry *m_registry;
    MaterialSystem *m_materials;
    LightingSystem* m_lights;
    std::unique_ptr<Shader> m_skyboxShader;
    Cubemap* m_skybox;
};

}