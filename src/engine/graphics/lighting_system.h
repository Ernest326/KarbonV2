#pragma once
#include <entt/entt.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

namespace Karbon {

// In std140, a float immediately following a vec3 occupies the vec3's 4-byte
// tail (the vec4 w-component slot). So vec3+float IS a valid packed vec4.
// glm::vec3 has sizeof=12 and alignof=4, so the C++ layout matches std140
// as long as we pair every vec3 with the float that follows it.
struct alignas(16) GPUPointLight {
    glm::vec3 position;    // bytes  0-11
    float     radius;      // bytes 12-15  (packs into vec3 tail — correct)
    glm::vec3 color;       // bytes 16-27
    float     intensity;   // bytes 28-31
    float     falloff;     // bytes 32-35
    float     padding[3];  // bytes 36-47
}; // 48 bytes
static_assert(sizeof(GPUPointLight) == 48);

struct alignas(16) GPUDirectionalLight {
    glm::vec3 direction;   // bytes  0-11
    float     intensity;   // bytes 12-15  (packs into vec3 tail — correct std140)
    glm::vec3 color;       // bytes 16-27
    float     padding;     // bytes 28-31
}; // 32 bytes
static_assert(sizeof(GPUDirectionalLight) == 32);

struct alignas(16) GPUSpotLight {
    glm::vec3 position;    // bytes  0-11
    float     cutOff;      // bytes 12-15  (packs into vec3 tail)
    glm::vec3 direction;   // bytes 16-27
    float     outerCutOff; // bytes 28-31  (packs into vec3 tail)
    glm::vec3 color;       // bytes 32-43
    float     intensity;   // bytes 44-47  (packs into vec3 tail)
    float     falloff;     // bytes 48-51
    float     padding[3];  // bytes 52-63
}; // 64 bytes
static_assert(sizeof(GPUSpotLight) == 64);

struct GPULightHeader {
    int pointCount;
    int directionalCount;
    int spotCount;
    int padding;
};

class LightingSystem {
public:
    static constexpr size_t MAX_POINT_LIGHTS = 64;
    static constexpr size_t MAX_DIRECTIONAL_LIGHTS = 8;
    static constexpr size_t MAX_SPOT_LIGHTS = 16;

    // RAII: the constructor creates the light UBO, the destructor deletes it
    LightingSystem(entt::registry *registry);
    ~LightingSystem();

    LightingSystem(const LightingSystem&) = delete;
    LightingSystem& operator=(const LightingSystem&) = delete;

    void update();

    GLuint getUBO() const { return m_lightsUBO; }

private:
    entt::registry *m_registry = nullptr;
    GLuint m_lightsUBO = 0;
    size_t m_uboSize = 0;

    std::vector<GPUPointLight> m_pointLights;
    std::vector<GPUDirectionalLight> m_directionalLights;
    std::vector<GPUSpotLight> m_spotLights;
};

} // namespace Karbon