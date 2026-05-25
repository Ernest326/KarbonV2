#pragma once
#include "texture.h"
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <glad/glad.h>

namespace Karbon {

struct GPUMaterial {
    glm::vec4 albedoColor;
    glm::vec4 emissiveColor;
    float metallic;
    float roughness;
    float hasAlbedoMap;
    float hasNormalMap;
    // Padded to 48 total bytes
};

using MaterialHandle = uint16_t;
constexpr MaterialHandle INVALID_MATERIAL = 0xFFFF;
constexpr size_t MAX_MATERIALS = 256;

class MaterialSystem {
public:
    MaterialSystem();
    ~MaterialSystem();

    MaterialHandle create(const glm::vec4& albedo, float metallic, float roughness);
    MaterialHandle createTextured(const glm::vec4& albedo, float metallic, float roughness, Texture* albedoMap=nullptr, Texture* normalMap=nullptr, Texture* roughnessMap=nullptr, Texture* metallicMap=nullptr, Texture* emissiveMap=nullptr);
    
    void update(MaterialHandle handle, const glm::vec4& albedo, float metallic, float roughness);
    void bindAlbedoMap(MaterialHandle handle, uint32_t slot);
    void bindNormalMap(MaterialHandle handle, uint32_t slot);
    void uploadToGPU();
    uint32_t getTextureBindingPoint() const { return 2; }
    GLuint getUBO() const { return m_materialUBO; }

private:
    std::array<GPUMaterial, MAX_MATERIALS> m_materials;
    std::array<Texture*, MAX_MATERIALS> m_albedoMaps;
    std::array<Texture*, MAX_MATERIALS> m_normalMaps;
    size_t m_count = 0;
    GLuint m_materialUBO = 0;
    bool m_dirty = true;
};


}