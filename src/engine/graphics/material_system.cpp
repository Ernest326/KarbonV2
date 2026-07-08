#include "material_system.h"


//TODO: Implement roughness/emissive/metallic maps

namespace Karbon {

MaterialSystem::MaterialSystem()
{
    glGenBuffers(1, &m_materialUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_materialUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GPUMaterial) * MAX_MATERIALS, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

MaterialSystem::~MaterialSystem()
{
    if (m_materialUBO) glDeleteBuffers(1, &m_materialUBO);
}

MaterialHandle MaterialSystem::create(const glm::vec4& albedo, float metallic, float roughness)
{
    if (m_count >= MAX_MATERIALS) return INVALID_MATERIAL;

    MaterialHandle handle = static_cast<MaterialHandle>(m_count++);
    m_materials[handle] = {albedo, glm::vec4(0.0f), metallic, roughness, 0.0f, 0.0f};
    m_albedoMaps[handle] = nullptr;
    m_normalMaps[handle] = nullptr;
    m_dirty = true;
    return handle;
}

MaterialHandle MaterialSystem::createTextured(const glm::vec4& albedo, float roughness, float metallic, Texture* albedoMap, Texture* normalMap, Texture* roughnessMap, Texture* metallicMap, Texture* emissiveMap, Texture* aoMap)
{
    if (m_count >= MAX_MATERIALS) return INVALID_MATERIAL;

    MaterialHandle handle = create(albedo, metallic, roughness);
    if (handle == INVALID_MATERIAL) return handle;
    
    m_albedoMaps[handle] = albedoMap;
    m_normalMaps[handle] = normalMap;
    m_roughnessMaps[handle] = roughnessMap;
    m_metallicMaps[handle] = metallicMap;
    m_emissiveMaps[handle] = emissiveMap;
    m_aoMaps[handle] = aoMap;

    m_materials[handle].hasAlbedoMap = albedoMap ? 1.0f : 0.0f;
    m_materials[handle].hasNormalMap = normalMap ? 1.0f : 0.0f;
    m_materials[handle].hasRoughnessMap = roughnessMap ? 1.0f : 0.0f;
    m_materials[handle].hasMetallicMap = metallicMap ? 1.0f : 0.0f;
    m_materials[handle].hasEmissiveMap = emissiveMap ? 1.0f : 0.0f;
    m_materials[handle].hasAOMap = aoMap ? 1.0f : 0.0f;

    m_dirty = true;
    return handle;
}

void MaterialSystem::update(MaterialHandle handle, const glm::vec4& albedo, float metallic, float roughness)
{
    if (handle >= m_count) return;
    m_materials[handle].albedoColor = albedo;
    m_materials[handle].metallic = metallic;
    m_materials[handle].roughness = roughness;
    m_dirty = true;
}

void MaterialSystem::bindMaterialTextures(MaterialHandle handle)
{
    if (handle >= m_count) return;
    if (m_albedoMaps[handle]) {
        m_albedoMaps[handle]->bind(ALBEDO_MAP_SLOT);
    }
    if (m_normalMaps[handle]) {
        m_normalMaps[handle]->bind(NORMAL_MAP_SLOT);
    }
    if (m_roughnessMaps[handle]) {
        m_roughnessMaps[handle]->bind(ROUGHNESS_MAP_SLOT);
    }
    if (m_metallicMaps[handle]) {
        m_metallicMaps[handle]->bind(METALLIC_MAP_SLOT);
    }
    if (m_emissiveMaps[handle]) {
        m_emissiveMaps[handle]->bind(EMISSIVE_MAP_SLOT);
    }
    if (m_aoMaps[handle]) {
        m_aoMaps[handle]->bind(AO_MAP_SLOT);
    }
}

void MaterialSystem::updateTexture(MaterialHandle handle, MaterialMap mapType, Texture* texture)
{
    if (handle >= m_count) return;

    switch (mapType) {
        case MaterialMap::Albedo:
            m_albedoMaps[handle] = texture;
            m_materials[handle].hasAlbedoMap = texture ? 1.0f : 0.0f;
            break;
        case MaterialMap::Normal:
            m_normalMaps[handle] = texture;
            m_materials[handle].hasNormalMap = texture ? 1.0f : 0.0f;
            break;
        case MaterialMap::Metallic:
            m_metallicMaps[handle] = texture;
            m_materials[handle].hasMetallicMap = texture ? 1.0f : 0.0f;
            break;
        case MaterialMap::Roughness:
            m_roughnessMaps[handle] = texture;
            m_materials[handle].hasRoughnessMap = texture ? 1.0f : 0.0f;
            break;
        case MaterialMap::Emissive:
            m_emissiveMaps[handle] = texture;
            m_materials[handle].hasEmissiveMap = texture ? 1.0f : 0.0f;
            break;
        case MaterialMap::AO:
            m_aoMaps[handle] = texture;
            m_materials[handle].hasAOMap = texture ? 1.0f : 0.0f;
            break;
    }
    m_dirty = true;
}

const GPUMaterial& MaterialSystem::getMaterialData(MaterialHandle handle) const
{
    static const GPUMaterial fallback{};
    if (handle >= m_count) return fallback;
    return m_materials[handle];
}

Texture* MaterialSystem::getTexture(MaterialHandle handle, MaterialMap mapType) const
{
    if (handle >= m_count) return nullptr;
    switch (mapType) {
        case MaterialMap::Albedo:    return m_albedoMaps[handle];
        case MaterialMap::Normal:    return m_normalMaps[handle];
        case MaterialMap::Metallic:  return m_metallicMaps[handle];
        case MaterialMap::Roughness: return m_roughnessMaps[handle];
        case MaterialMap::Emissive:  return m_emissiveMaps[handle];
        case MaterialMap::AO:        return m_aoMaps[handle];
    }
    return nullptr;
}

void MaterialSystem::uploadToGPU()
{
    if (!m_dirty) return;

    glBindBuffer(GL_UNIFORM_BUFFER, m_materialUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GPUMaterial) * m_count, m_materials.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    m_dirty = false;
}

}