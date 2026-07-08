#include "assets/material_asset.h"
#include "utils/logger.h"
#include <fstream>
#include <sstream>

namespace Karbon {

namespace {

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Leaves components at their fallback value if the stream runs out early
glm::vec4 parseVec4(const std::string& value, const glm::vec4& fallback) {
    std::istringstream iss(value);
    glm::vec4 result = fallback;
    iss >> result.x >> result.y >> result.z >> result.w;
    return result;
}

float parseFloat(const std::string& value, float fallback) {
    try {
        return std::stof(value);
    } catch (const std::exception&) {
        return fallback;
    }
}

}

bool loadMaterialAsset(const std::string& path, MaterialAsset& outAsset) {
    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::get().error("MaterialAsset: failed to open '" + path + "'");
        return false;
    }

    MaterialAsset asset; // defaults; the file only overrides keys it specifies
    std::string line;
    while (std::getline(file, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#') continue;

        size_t eq = trimmed.find('=');
        if (eq == std::string::npos) continue;

        std::string key = trim(trimmed.substr(0, eq));
        std::string value = trim(trimmed.substr(eq + 1));

        if (key == "albedo") asset.albedo = parseVec4(value, asset.albedo);
        else if (key == "emissive") asset.emissive = parseVec4(value, asset.emissive);
        else if (key == "metallic") asset.metallic = parseFloat(value, asset.metallic);
        else if (key == "roughness") asset.roughness = parseFloat(value, asset.roughness);
        else if (key == "albedoMap") asset.albedoMap = value;
        else if (key == "normalMap") asset.normalMap = value;
        else if (key == "metallicMap") asset.metallicMap = value;
        else if (key == "roughnessMap") asset.roughnessMap = value;
        else if (key == "emissiveMap") asset.emissiveMap = value;
        else if (key == "aoMap") asset.aoMap = value;
    }

    outAsset = asset;
    return true;
}

bool saveMaterialAsset(const std::string& path, const MaterialAsset& asset) {
    std::ofstream file(path);
    if (!file.is_open()) {
        Logger::get().error("MaterialAsset: failed to write '" + path + "'");
        return false;
    }

    auto writeVec4 = [&file](const char* key, const glm::vec4& v) {
        file << key << " = " << v.x << " " << v.y << " " << v.z << " " << v.w << "\n";
    };

    file << "# Karbon material asset\n";
    writeVec4("albedo", asset.albedo);
    writeVec4("emissive", asset.emissive);
    file << "metallic = " << asset.metallic << "\n";
    file << "roughness = " << asset.roughness << "\n";
    file << "albedoMap = " << asset.albedoMap << "\n";
    file << "normalMap = " << asset.normalMap << "\n";
    file << "metallicMap = " << asset.metallicMap << "\n";
    file << "roughnessMap = " << asset.roughnessMap << "\n";
    file << "emissiveMap = " << asset.emissiveMap << "\n";
    file << "aoMap = " << asset.aoMap << "\n";
    return true;
}

}
