#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "graphics/buffers/buffers.h"
#include "graphics/material_system.h"

namespace Karbon {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec3 tangent;
    Vertex(glm::vec3 p=glm::vec3(0.0f), glm::vec3 n=glm::vec3(0.0f), glm::vec2 t=glm::vec2(0.0f), glm::vec3 tan=glm::vec3(0.0f)) : position(p), normal(n), texCoord(t), tangent(tan) {}
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    std::unique_ptr<VAO> vao;
    std::unique_ptr<VBO> vbo;
    std::unique_ptr<IBO> ibo; 

    MaterialHandle material;

    void uploadToGPU();
    void draw() const;
};

}