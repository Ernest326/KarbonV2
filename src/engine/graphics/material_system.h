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
    float roughness;
    float metallic;
    float hasAlbedoMap;
    float hasNormalMap;
    float hasRoughnessMap;
    float hasMetallicMap;
    float hasAOMap;
    float hasEmissiveMap;
};

using MaterialHandle = uint16_t;
constexpr MaterialHandle INVALID_MATERIAL = 0xFFFF;
constexpr size_t MAX_MATERIALS = 256;

constexpr uint32_t ALBEDO_MAP_SLOT = 0;
constexpr uint32_t NORMAL_MAP_SLOT    = 1;
constexpr uint32_t METALLIC_MAP_SLOT   = 2;
constexpr uint32_t ROUGHNESS_MAP_SLOT  = 3;
constexpr uint32_t AO_MAP_SLOT         = 4;
constexpr uint32_t EMISSIVE_MAP_SLOT   = 5;

class MaterialSystem {
public:
    MaterialSystem();
    ~MaterialSystem();

    MaterialHandle create(const glm::vec4& albedo, float metallic, float roughness);
    MaterialHandle createTextured(const glm::vec4& albedo, float roughness, float metallic, Texture* albedoMap=nullptr, Texture* normalMap=nullptr, Texture* roughnessMap=nullptr, Texture* metallicMap=nullptr, Texture* emissiveMap=nullptr, Texture* aoMap=nullptr);
    
    void update(MaterialHandle handle, const glm::vec4& albedo, float metallic, float roughness);
    
    void bindMaterialTextures(MaterialHandle handle);

    enum class MaterialMap { Albedo, Normal, Metallic, Roughness, Emissive, AO };
    void updateTexture(MaterialHandle handle, MaterialMap mapType, Texture* texture);

    void uploadToGPU();
    GLuint getUBO() const { return m_materialUBO; }

private:
    std::array<GPUMaterial, MAX_MATERIALS> m_materials;
    std::array<Texture*, MAX_MATERIALS> m_albedoMaps;
    std::array<Texture*, MAX_MATERIALS> m_normalMaps;
    std::array<Texture*, MAX_MATERIALS> m_metallicMaps;
    std::array<Texture*, MAX_MATERIALS> m_roughnessMaps;
    std::array<Texture*, MAX_MATERIALS> m_emissiveMaps;
    std::array<Texture*, MAX_MATERIALS> m_aoMaps;

    size_t m_count = 0;
    GLuint m_materialUBO = 0;
    bool m_dirty = true;
};


}