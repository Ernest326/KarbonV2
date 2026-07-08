#pragma once
#include "assets/asset_handles.h"
#include "assets/material_asset.h"
#include "scene/mesh.h"
#include "graphics/texture.h"
#include "graphics/material_system.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Karbon {

// Where a mesh came from — enough information to reload it when a scene is deserialized
struct MeshSource {
    enum class Type { Primitive, Model };
    Type type = Type::Primitive;
    std::string path;          // primitive name ("cube") or model file path
    uint32_t subMeshIndex = 0; // mesh index within a model file
};

// Owns all meshes and textures and hands out stable handles.
// Components store handles (serializable), never raw pointers.
class AssetManager {
public:
    explicit AssetManager(MaterialSystem* materials);

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // Primitive meshes: lazily created, one shared instance each
    MeshHandle getCubeMesh();
    MeshHandle getSphereMesh();
    MeshHandle getPlaneMesh();

    // Loads all meshes from a model file (cached by path).
    // Creates materials and textures through the material system as encountered.
    std::vector<MeshHandle> loadModel(const std::string& path);

    Mesh* getMesh(MeshHandle handle);
    const MeshSource* getMeshSource(MeshHandle handle) const;

    // Textures: cached by path, deduplicated
    TextureHandle loadTexture(const std::string& path);
    Texture* getTexture(TextureHandle handle);
    const std::string* getTexturePath(TextureHandle handle) const;

    // Materials: loads a .kmat file (cached by path), resolving referenced
    // texture paths through the texture cache above.
    MaterialHandle loadMaterial(const std::string& path);

    // Writes the material behind `handle` out to `path` as a .kmat file;
    // texture slots are recovered from their cached load paths (textures
    // not loaded through this AssetManager are written as empty slots).
    bool saveMaterial(const std::string& path, MaterialHandle handle);

    // Reverse lookup: the path a material was loaded from, if it was loaded
    // via loadMaterial(). Empty for materials created directly through
    // MaterialSystem::create()/createTextured() with no backing file.
    std::string getMaterialPath(MaterialHandle handle) const;

    MaterialSystem* getMaterialSystem() { return m_materials; }

private:
    MeshHandle addMesh(std::unique_ptr<Mesh> mesh, MeshSource source);
    std::string findTexturePath(Texture* texture) const;

    struct MeshEntry {
        std::unique_ptr<Mesh> mesh;
        MeshSource source;
    };
    struct TextureEntry {
        std::unique_ptr<Texture> texture;
        std::string path;
    };

    std::vector<MeshEntry> m_meshes;
    std::vector<TextureEntry> m_textures;

    std::unordered_map<std::string, std::vector<MeshHandle>> m_modelCache;
    std::unordered_map<std::string, TextureHandle> m_textureCache;
    std::unordered_map<std::string, MaterialHandle> m_materialCache;

    MeshHandle m_cubeMesh = INVALID_MESH;
    MeshHandle m_sphereMesh = INVALID_MESH;
    MeshHandle m_planeMesh = INVALID_MESH;

    MaterialSystem* m_materials = nullptr;
};

}
