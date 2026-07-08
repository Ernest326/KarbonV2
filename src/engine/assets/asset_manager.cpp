#include "assets/asset_manager.h"
#include "scene/cube_mesh.h"
#include "scene/sphere_mesh.h"
#include "scene/plane_mesh.h"
#include "scene/model.h"
#include "utils/logger.h"

namespace Karbon {

AssetManager::AssetManager(MaterialSystem* materials) : m_materials(materials) {}

MeshHandle AssetManager::addMesh(std::unique_ptr<Mesh> mesh, MeshSource source) {
    m_meshes.push_back({std::move(mesh), std::move(source)});
    return static_cast<MeshHandle>(m_meshes.size() - 1);
}

MeshHandle AssetManager::getCubeMesh() {
    if (m_cubeMesh == INVALID_MESH) {
        // Slice-move: the primitive classes only fill the base Mesh in their
        // constructor and add no members, so moving into a plain Mesh is safe.
        CubeMesh cube;
        m_cubeMesh = addMesh(std::make_unique<Mesh>(std::move(cube)),
                             {MeshSource::Type::Primitive, "cube", 0});
    }
    return m_cubeMesh;
}

MeshHandle AssetManager::getSphereMesh() {
    if (m_sphereMesh == INVALID_MESH) {
        SphereMesh sphere;
        m_sphereMesh = addMesh(std::make_unique<Mesh>(std::move(sphere)),
                               {MeshSource::Type::Primitive, "sphere", 0});
    }
    return m_sphereMesh;
}

MeshHandle AssetManager::getPlaneMesh() {
    if (m_planeMesh == INVALID_MESH) {
        PlaneMesh plane;
        m_planeMesh = addMesh(std::make_unique<Mesh>(std::move(plane)),
                              {MeshSource::Type::Primitive, "plane", 0});
    }
    return m_planeMesh;
}

std::vector<MeshHandle> AssetManager::loadModel(const std::string& path) {
    auto it = m_modelCache.find(path);
    if (it != m_modelCache.end()) {
        return it->second;
    }

    Model model(path.c_str(), m_materials, this);
    std::vector<Mesh> meshes = model.releaseMeshes();

    std::vector<MeshHandle> handles;
    handles.reserve(meshes.size());
    for (size_t i = 0; i < meshes.size(); ++i) {
        handles.push_back(addMesh(std::make_unique<Mesh>(std::move(meshes[i])),
                                  {MeshSource::Type::Model, path, static_cast<uint32_t>(i)}));
    }

    if (handles.empty()) {
        Logger::get().error("AssetManager: no meshes loaded from model '" + path + "'");
    }
    m_modelCache.emplace(path, handles);
    return handles;
}

Mesh* AssetManager::getMesh(MeshHandle handle) {
    if (handle >= m_meshes.size()) return nullptr;
    return m_meshes[handle].mesh.get();
}

const MeshSource* AssetManager::getMeshSource(MeshHandle handle) const {
    if (handle >= m_meshes.size()) return nullptr;
    return &m_meshes[handle].source;
}

TextureHandle AssetManager::loadTexture(const std::string& path) {
    auto it = m_textureCache.find(path);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    m_textures.push_back({std::make_unique<Texture>(path.c_str()), path});
    TextureHandle handle = static_cast<TextureHandle>(m_textures.size() - 1);
    m_textureCache.emplace(path, handle);
    return handle;
}

Texture* AssetManager::getTexture(TextureHandle handle) {
    if (handle >= m_textures.size()) return nullptr;
    return m_textures[handle].texture.get();
}

const std::string* AssetManager::getTexturePath(TextureHandle handle) const {
    if (handle >= m_textures.size()) return nullptr;
    return &m_textures[handle].path;
}

MaterialHandle AssetManager::loadMaterial(const std::string& path) {
    auto it = m_materialCache.find(path);
    if (it != m_materialCache.end()) {
        return it->second;
    }

    MaterialAsset asset;
    if (!loadMaterialAsset(path, asset)) {
        return INVALID_MATERIAL;
    }

    auto resolve = [this](const std::string& texPath) -> Texture* {
        if (texPath.empty()) return nullptr;
        return getTexture(loadTexture(texPath));
    };

    // Argument order must match MaterialSystem::createTextured exactly:
    // (albedo, roughness, metallic, albedoMap, normalMap, roughnessMap, metallicMap, emissiveMap, aoMap)
    MaterialHandle handle = m_materials->createTextured(
        asset.albedo, asset.roughness, asset.metallic,
        resolve(asset.albedoMap), resolve(asset.normalMap),
        resolve(asset.roughnessMap), resolve(asset.metallicMap),
        resolve(asset.emissiveMap), resolve(asset.aoMap));

    m_materialCache.emplace(path, handle);
    return handle;
}

std::string AssetManager::findTexturePath(Texture* texture) const {
    if (!texture) return "";
    for (const auto& entry : m_textures) {
        if (entry.texture.get() == texture) return entry.path;
    }
    return "";
}

bool AssetManager::saveMaterial(const std::string& path, MaterialHandle handle) {
    const GPUMaterial& data = m_materials->getMaterialData(handle);

    MaterialAsset asset;
    asset.albedo = data.albedoColor;
    asset.emissive = data.emissiveColor;
    asset.metallic = data.metallic;
    asset.roughness = data.roughness;
    asset.albedoMap    = findTexturePath(m_materials->getTexture(handle, MaterialSystem::MaterialMap::Albedo));
    asset.normalMap    = findTexturePath(m_materials->getTexture(handle, MaterialSystem::MaterialMap::Normal));
    asset.metallicMap  = findTexturePath(m_materials->getTexture(handle, MaterialSystem::MaterialMap::Metallic));
    asset.roughnessMap = findTexturePath(m_materials->getTexture(handle, MaterialSystem::MaterialMap::Roughness));
    asset.emissiveMap  = findTexturePath(m_materials->getTexture(handle, MaterialSystem::MaterialMap::Emissive));
    asset.aoMap        = findTexturePath(m_materials->getTexture(handle, MaterialSystem::MaterialMap::AO));

    return saveMaterialAsset(path, asset);
}

std::string AssetManager::getMaterialPath(MaterialHandle handle) const {
    for (const auto& [path, cachedHandle] : m_materialCache) {
        if (cachedHandle == handle) return path;
    }
    return "";
}

}
