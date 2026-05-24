#pragma once
#include <glm/glm.hpp>
#include "../graphics/texture.h"
#include <memory>

namespace Karbon {

class Texture;

//PBR Material struct
struct Material {
    glm::vec4 albedoColor = glm::vec4(1.0f);
    std::shared_ptr<Texture> albedoTexture = nullptr;

    float metallic = 0.0f; 
    float roughness = 0.5f;
    std::shared_ptr<Texture> roughnessTexture = nullptr;
    std::shared_ptr<Texture> metallicTexture = nullptr;
    std::shared_ptr<Texture> normalTexture = nullptr;
    std::shared_ptr<Texture> aoTexture = nullptr;
    std::shared_ptr<Texture> heightTexture = nullptr;

    glm::vec4 emissiveColor = glm::vec4(0.0f);
    std::shared_ptr<Texture> emissiveTexture = nullptr;

    bool alphaTest = false;
    float alphaCutoff = 0.5f;
};

using MaterialHandle = size_t;

}