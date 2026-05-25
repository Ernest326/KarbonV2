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

MaterialHandle MaterialSystem::createTextured(const glm::vec4& albedo, float metallic, float roughness, Texture* albedoMap, Texture* normalMap, Texture* roughnessMap, Texture* metallicMap, Texture* emissiveMap)
{
    if (m_count >= MAX_MATERIALS) return INVALID_MATERIAL;

    MaterialHandle handle = create(albedo, metallic, roughness);
    if (handle == INVALID_MATERIAL) return handle;
    
    m_albedoMaps[handle] = albedoMap;
    m_normalMaps[handle] = normalMap;
    m_materials[handle].hasAlbedoMap = albedoMap ? 1.0f : 0.0f;
    m_materials[handle].hasNormalMap = normalMap ? 1.0f : 0.0f;
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

void MaterialSystem::bindAlbedoMap(MaterialHandle handle, uint32_t slot)
{
    if (handle >= m_count) return;
    if (m_albedoMaps[handle]) {
        m_albedoMaps[handle]->bind(slot);
    } 
}

void MaterialSystem::bindNormalMap(MaterialHandle handle, uint32_t slot)
{
    if (handle >= m_count) return;
    if (m_normalMaps[handle]) {
        m_normalMaps[handle]->bind(slot);
    } 
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