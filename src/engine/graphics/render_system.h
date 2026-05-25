#pragma once
#include <entt/entt.hpp>
#include "shader.h"
#include "material_system.h"
#include "lighting_system.h"

namespace Karbon {

class RenderSystem {

public:
    RenderSystem(entt::registry *registry, MaterialSystem *materials, LightingSystem *lights);
    ~RenderSystem();
    void Draw(Shader* shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos);

private:
    entt::registry *m_registry;
    MaterialSystem *m_materials;
    LightingSystem* m_lights;
};

}