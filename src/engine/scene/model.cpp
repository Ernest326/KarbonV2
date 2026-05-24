#include "model.h"
#include "assimp/material.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>

namespace Karbon {

Model::Model(const char* filepath) { loadModel(filepath); }

void Model::draw() const {
    for (const auto& mesh : m_meshes) {
        mesh.draw();
    }
}

void Model::loadModel(const char* filepath) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    processAiNode(scene->mRootNode, scene);
}

void Model::processAiNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(processAiMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processAiNode(node->mChildren[i], scene);
    }
}

Mesh Model::processAiMesh(aiMesh* aiMesh, const aiScene* scene) {
    Mesh mesh;

    for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.position = glm::vec3(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);
        if (aiMesh->HasNormals()) {
            vertex.normal = glm::vec3(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z);
        } else {
            vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        if (aiMesh->HasTextureCoords(0)) {
            vertex.texCoord = glm::vec2(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y);
        } else {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }
        mesh.vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
        aiFace face = aiMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            mesh.indices.push_back(face.mIndices[j]);
        }
    }

    if (aiMesh->mMaterialIndex >= 0) {
        mesh.material = processAiMaterial(scene->mMaterials[aiMesh->mMaterialIndex], scene);
    }
    return mesh;
}

MaterialHandle Model::processAiMaterial(aiMaterial* aiMaterial, const aiScene* scene) {
    Material material;
    aiColor3D color(0.0f, 0.0f, 0.0f);
    
    if (AI_SUCCESS == aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
        material.albedoColor = glm::vec4(color.r, color.g, color.b, 1.0f);
    }

    float metallic, roughness;
    if (AI_SUCCESS == aiMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic)) {
        material.metallic = metallic;
    }
    if (AI_SUCCESS == aiMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness)) {
        material.roughness = roughness;
    }

    m_materials.push_back(material);
    return m_materials.size() - 1;
}

}