#include "lighting_system.h"
#include "../scene/components/pointlight_component.h"
#include "../scene/components/directional_light_component.h"
#include "../scene/components/spotlight_component.h"
#include "../scene/components/transform.h"
#include <iostream>

namespace Karbon {

LightingSystem::LightingSystem(entt::registry *registry)
    : m_registry(registry) {}

LightingSystem::~LightingSystem() {
    if (m_lightsUBO)
        glDeleteBuffers(1, &m_lightsUBO);
}

void LightingSystem::Initialize() {

    m_uboSize = sizeof(GPULightHeader) + sizeof(GPUPointLight) * MAX_POINT_LIGHTS +
                sizeof(GPUDirectionalLight) * MAX_DIRECTIONAL_LIGHTS +
                sizeof(GPUSpotLight) * MAX_SPOT_LIGHTS;

    // Create UBO
    glGenBuffers(1, &m_lightsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_lightsUBO);
    glBufferData(GL_UNIFORM_BUFFER, m_uboSize, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void LightingSystem::Update() {

    m_pointLights.clear();
    m_directionalLights.clear();
    m_spotLights.clear();

    // Gather point lights
    auto view = m_registry->view<WorldTransformComponent, PointLightComponent>();
    for (auto entity : view) {
        auto &transform = view.get<WorldTransformComponent>(entity);
        auto &light = view.get<PointLightComponent>(entity);

        if (m_pointLights.size() >= MAX_POINT_LIGHTS)
            break;

        GPUPointLight gpuLight;
        gpuLight.position = transform.worldPosition;
        gpuLight.color = light.color;
        gpuLight.intensity = light.intensity;
        gpuLight.radius = light.radius;
        gpuLight.falloff = light.falloff;
        gpuLight.padding[0] = 0.0f;
        gpuLight.padding[1] = 0.0f;
        gpuLight.padding[2] = 0.0f;

        m_pointLights.push_back(gpuLight);
        light.dirty = false;
    }

    // Gather directional lights
    auto dirView = m_registry->view<WorldTransformComponent, DirectionalLightComponent>();
    for (auto entity : dirView) {
        auto &transform = dirView.get<WorldTransformComponent>(entity);
        auto &light = dirView.get<DirectionalLightComponent>(entity);

        if (m_directionalLights.size() >= MAX_DIRECTIONAL_LIGHTS)
            break;

        GPUDirectionalLight directionalLight;
        directionalLight.direction = transform.forward();
        directionalLight.intensity = light.intensity;
        directionalLight.color     = light.color;
        directionalLight.padding   = 0.0f;

        m_directionalLights.push_back(directionalLight);
        light.dirty = false;
    }

    // Gather spot lights
    auto spotView = m_registry->view<WorldTransformComponent, SpotLightComponent>();
    for (auto entity : spotView) {
        auto &transform = spotView.get<WorldTransformComponent>(entity);
        auto &light = spotView.get<SpotLightComponent>(entity);

        if (m_spotLights.size() >= MAX_SPOT_LIGHTS)
            break;

        GPUSpotLight spotLight;
        spotLight.position = transform.worldPosition;
        spotLight.direction = transform.forward();
        spotLight.color = light.color;
        spotLight.intensity = light.intensity;
        spotLight.falloff = light.falloff;
        spotLight.cutOff = light.cutOff;
        spotLight.outerCutOff = light.outerCutOff;
        spotLight.padding[0] = 0.0f;
        spotLight.padding[1] = 0.0f;
        spotLight.padding[2] = 0.0f;

        m_spotLights.push_back(spotLight);
        light.dirty = false;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, m_lightsUBO);

    GPULightHeader header;
    header.pointCount = static_cast<int>(m_pointLights.size());
    header.directionalCount = static_cast<int>(m_directionalLights.size());
    header.spotCount = static_cast<int>(m_spotLights.size());
    header.padding = 0;

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GPULightHeader), &header);
    size_t offset = sizeof(GPULightHeader);

    if (!m_pointLights.empty()) {
        glBufferSubData(GL_UNIFORM_BUFFER, offset,
                      m_pointLights.size() * sizeof(GPUPointLight),
                      m_pointLights.data());
    }
    offset += MAX_POINT_LIGHTS * sizeof(GPUPointLight);

    if (!m_directionalLights.empty()) {
        glBufferSubData(GL_UNIFORM_BUFFER, offset,
                      m_directionalLights.size() * sizeof(GPUDirectionalLight),
                      m_directionalLights.data());
    }
    offset += MAX_DIRECTIONAL_LIGHTS * sizeof(GPUDirectionalLight);

    if (!m_spotLights.empty()) {
        glBufferSubData(GL_UNIFORM_BUFFER, offset,
                      m_spotLights.size() * sizeof(GPUSpotLight),
                      m_spotLights.data());
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