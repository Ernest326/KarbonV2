#pragma once
#include <entt/entt.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

namespace Karbon {

struct GPUPointLight {
    glm::vec3 position;
    float radius;
    glm::vec3 color;
    float intensity;
    float falloff;
    float padding[3];
};

class LightingSystem {
public:
    static constexpr size_t MAX_POINT_LIGHTS = 64;

    LightingSystem(entt::registry *registry);
    ~LightingSystem();

    void Initialize();
    void Update();
    void Shutdown();

    GLuint getUBO() const { return m_lightsUBO; }
    size_t getActiveLightCount() const { return m_activeLightCount; }

private:
    entt::registry *m_registry = nullptr;
    GLuint m_lightsUBO = 0;
    size_t m_activeLightCount;
    std::vector<GPUPointLight> m_gpuLights;
};

} // namespace Karbon