#pragma once
#include "mesh.h"
#include "graphics/material_system.h"
#include <string>
#include <vector>
#include <assimp/scene.h>

namespace Karbon {

class AssetManager;

class Model {
public:
    // assets is used to load and own referenced textures (may be null: textures skipped)
    Model(const char* filepath, MaterialSystem* materialSystem, AssetManager* assets = nullptr);
    ~Model() = default;

    void draw() const;

    inline const std::vector<Mesh>& getMeshes() const { return m_meshes; }
    inline Mesh& getMesh(size_t index) { return m_meshes.at(index); }

    // Transfers mesh ownership to the caller (used by AssetManager)
    std::vector<Mesh> releaseMeshes() { return std::move(m_meshes); }

private:
    std::vector<Mesh> m_meshes;
    AssetManager* m_assets = nullptr;

    void loadModel(const char* filepath, MaterialSystem* materialSystem);
    void processAiNode(aiNode* node, const aiScene* scene, MaterialSystem* materialSystem, const glm::mat4& parentTransform);
    Mesh processAiMesh(aiMesh* mesh, const aiScene* scene, MaterialSystem* materialSystem, const glm::mat4& transform);
    MaterialHandle processAiMaterial(aiMaterial* material, const aiScene* scene, MaterialSystem* materialSystem);
};

}