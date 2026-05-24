#include "lighting_system.h"
#include "../scene/components/pointlight_component.h"
#include "../scene/components/transform.h"

namespace Karbon {

LightingSystem::LightingSystem(entt::registry *registry)
    : m_registry(registry) {}

LightingSystem::~LightingSystem() {
    if (m_lightsUBO)
        glDeleteBuffers(1, &m_lightsUBO);
}

void LightingSystem::Initialize() {

    // Create UBO
    glGenBuffers(1, &m_lightsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_lightsUBO);
    glBufferData(GL_UNIFORM_BUFFER, 16 + sizeof(GPUPointLight) * MAX_POINT_LIGHTS,
               nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void LightingSystem::Update() {
    m_gpuLights.clear();
    m_activeLightCount = 0;

    auto view = m_registry->view<TransformComponent, PointLightComponent>();
    for (auto entity : view) {
        auto &transform = view.get<TransformComponent>(entity);
        auto &light = view.get<PointLightComponent>(entity);

        if (m_activeLightCount >= MAX_POINT_LIGHTS)
            break;

        GPUPointLight gpuLight;
        gpuLight.position = transform.position;
        gpuLight.color = light.color;
        gpuLight.intensity = light.intensity;
        gpuLight.radius = light.radius;
        gpuLight.falloff = light.falloff;
        gpuLight.padding[0] = 0.0f;
        gpuLight.padding[1] = 0.0f;
        gpuLight.padding[2] = 0.0f;

        m_gpuLights.push_back(gpuLight);
        m_activeLightCount++;
        light.dirty = false;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, m_lightsUBO);
    int lightCount = static_cast<int>(m_activeLightCount);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(int), &lightCount);
    if (!m_gpuLights.empty()) {
        glBufferSubData(GL_UNIFORM_BUFFER, 16,
                      m_gpuLights.size() * sizeof(GPUPointLight),
                      m_gpuLights.data());
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void LightingSystem::Shutdown() {
    if (m_lightsUBO) {
        glDeleteBuffers(1, &m_lightsUBO);
        m_lightsUBO = 0;
    }
}

} // namespace Karbon