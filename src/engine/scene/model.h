#pragma once
#include "mesh.h"
#include "../graphics/material_system.h"
#include <string>
#include <vector>
#include <assimp/scene.h>

namespace Karbon {

class Model {
public:
    explicit Model(const char* filepath, MaterialSystem* materialSystem);
    ~Model() = default;
    
    void draw() const;
    
    inline const std::vector<Mesh>& getMeshes() const { return m_meshes; }

private:
    std::vector<Mesh> m_meshes;

    void loadModel(const char* filepath, MaterialSystem* materialSystem);
    void processAiNode(aiNode* node, const aiScene* scene, MaterialSystem* materialSystem);
    Mesh processAiMesh(aiMesh* mesh, const aiScene* scene, MaterialSystem* materialSystem);
    MaterialHandle processAiMaterial(aiMaterial* material, const aiScene* scene, MaterialSystem* materialSystem);
};

}