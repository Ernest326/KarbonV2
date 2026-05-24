#pragma once
#include "material.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "../graphics/buffers/buffers.h"

namespace Karbon {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec3 tangent;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Material> materials;

    std::unique_ptr<VAO> vao;
    std::unique_ptr<VBO> vbo;
    std::unique_ptr<IBO> ibo; 
    MaterialHandle material = 0;

    void uploadToGPU();
    void draw() const;
};

}