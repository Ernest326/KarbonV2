#include "model.h"
#include "assimp/material.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>

namespace Karbon {

Model::Model(const char* filepath, MaterialSystem* materialSystem) { loadModel(filepath, materialSystem); }

void Model::draw() const {
    for (const auto& mesh : m_meshes) {
        mesh.draw();
    }
}

void Model::loadModel(const char* filepath, MaterialSystem* materialSystem) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    processAiNode(scene->mRootNode, scene, materialSystem);
}

void Model::processAiNode(aiNode* node, const aiScene* scene, MaterialSystem* materialSystem) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(processAiMesh(mesh, scene, materialSystem));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processAiNode(node->mChildren[i], scene, materialSystem);
    }
}

Mesh Model::processAiMesh(aiMesh* aiMesh, const aiScene* scene, MaterialSystem* materialSystem) {
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
        mesh.material = processAiMaterial(scene->mMaterials[aiMesh->mMaterialIndex], scene, materialSystem);
    }
    return mesh;
}

MaterialHandle Model::processAiMaterial(aiMaterial* aiMaterial, const aiScene* scene, MaterialSystem* materialSystem) {
    if (!materialSystem) return 0;

    aiColor3D color(1.0f, 1.0f, 1.0f);
    aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);

    float metallic = 0.0f;
    float roughness = 0.5f;
    aiMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
    aiMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);

    glm::vec4 albedo = glm::vec4(color.r, color.g, color.b, 1.0f);

    Texture* albedoMap = nullptr;
    Texture* normalMap = nullptr;
    Texture* metallicMap = nullptr;
    Texture* roughnessMap = nullptr;

    if (aiMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString path;
        aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path);
        albedoMap = new Texture(path.C_Str());
    }
    if (aiMaterial->GetTextureCount(aiTextureType_NORMALS) > 0) {
        aiString path;
        aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &path);
        normalMap = new Texture(path.C_Str());
    }
    if (aiMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0) {
        aiString path;
        aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &path);
        metallicMap = new Texture(path.C_Str());
    }
    if (aiMaterial->GetTextureCount(aiTextureType_SHININESS) > 0) {
        aiString path;
        aiMaterial->GetTexture(aiTextureType_SHININESS, 0, &path);
        roughnessMap = new Texture(path.C_Str());
    }

    return materialSystem->createTextured(albedo, metallic, roughness, albedoMap, normalMap, metallicMap, roughnessMap);
    
}

}