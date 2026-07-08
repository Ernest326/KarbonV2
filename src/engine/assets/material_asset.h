#pragma once
#include <glm/glm.hpp>
#include <string>

namespace Karbon {

// Plain-data description of a material, loaded from / saved to a simple
// text asset file (.kmat). Texture fields are paths (project-relative,
// matching how meshes/textures are already loaded); empty means "no
// texture in that slot".
struct MaterialAsset {
    glm::vec4 albedo{1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 emissive{0.0f, 0.0f, 0.0f, 0.0f};
    float metallic = 0.0f;
    float roughness = 0.5f;

    std::string albedoMap;
    std::string normalMap;
    std::string metallicMap;
    std::string roughnessMap;
    std::string emissiveMap;
    std::string aoMap;
};

// Parses a "key = value" text file (# starts a comment, blank lines
// ignored). Returns false and logs if the file can't be opened.
bool loadMaterialAsset(const std::string& path, MaterialAsset& outAsset);

// Writes a MaterialAsset back out in the same format.
bool saveMaterialAsset(const std::string& path, const MaterialAsset& asset);

}
