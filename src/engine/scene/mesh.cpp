#include "mesh.h"
#include <glad/glad.h>

namespace Karbon {

void Mesh::uploadToGPU(){
    if (vertices.empty() || indices.empty()) return;

    vbo = std::make_unique<VBO>(vertices.data(), vertices.size() * sizeof(Vertex));
    ibo = std::make_unique<IBO>(indices.data(), indices.size());
    vao = std::make_unique<VAO>();

    vao->bind();
    vao->addBuffer(*vbo, 0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    vao->addBuffer(*vbo, 1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, normal));
    vao->addBuffer(*vbo, 2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, texCoord));

    vao->unbind();
    vbo->unbind();
    ibo->unbind();
} 

void Mesh::draw() const{
    vao->bind();
    ibo->bind();
    glDrawElements(GL_TRIANGLES, ibo->getCount(), GL_UNSIGNED_INT, nullptr);
    ibo->unbind();
    vao->unbind();
} 

}