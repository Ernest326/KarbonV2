#pragma once
#include "mesh.h"
#include <string>
#include <vector>
#include <assimp/scene.h>

namespace Karbon {

class Model {
public:
    explicit Model(const char* filepath);
    ~Model() = default;
    
    void draw() const;
    
    inline const std::vector<Mesh>& getMeshes() const { return m_meshes; }

private:
    std::vector<Mesh> m_meshes;
    std::vector<Material> m_materials;

    void loadModel(const char* filepath);
    void processAiNode(aiNode* node, const aiScene* scene);
    Mesh processAiMesh(aiMesh* mesh, const aiScene* scene);
    MaterialHandle processAiMaterial(aiMaterial* material, const aiScene* scene);
};

}