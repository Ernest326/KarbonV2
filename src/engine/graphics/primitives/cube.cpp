#include "cube.h"
#include <glm/gtc/quaternion.hpp>

namespace Karbon {

std::unique_ptr<VBO> Cube::vertexBuffer = nullptr;
std::unique_ptr<IBO> Cube::indexBuffer = nullptr;
std::unique_ptr<VBO> Cube::texCoordBuffer = nullptr;
std::unique_ptr<VAO> Cube::vertexArray = nullptr;

Cube::Cube(glm::vec3 position, glm::quat rotation, glm::vec3 scale) : position(position), rotation(rotation), scale(scale) {
    
    static bool buffersInitialized = false;
    if (!buffersInitialized) {
        GLfloat* cube_verts = new GLfloat[108] {
            // Front face
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            // Back face
            -0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            // Left face
            -0.5f, -0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            // Right face
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
            // Top face
            -0.5f,  0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            // Bottom face
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f
        };

        GLuint indices[36] = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4,
            8, 9, 10, 10, 11, 8,
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
            20, 21, 22, 22, 23, 20
        };

        GLfloat* glTexCoords = new GLfloat[72] {
            // Front face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            // Back face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            // Left face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            // Right face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            // Top face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            // Bottom face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
        };

        buffersInitialized = true;
        this->vertexBuffer = std::make_unique<VBO>(cube_verts, 108*sizeof(GLfloat));
        this->indexBuffer = std::make_unique<IBO>(indices, 36);
        this->texCoordBuffer = std::make_unique<VBO>(glTexCoords, 72*sizeof(GLfloat));
        this->vertexArray = std::make_unique<VAO>();
        this->vertexArray->addBuffer(*this->vertexBuffer, 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
        this->vertexArray->addBuffer(*this->texCoordBuffer, 1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);
        delete[] cube_verts;
        delete[] glTexCoords;
    }    
}

glm::mat4 Cube::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model *= glm::mat4_cast(rotation);
    model = glm::scale(model, scale);
    return model;
}

void Cube::draw() const {
    vertexArray->bind();
    indexBuffer->bind();
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    indexBuffer->unbind();
    vertexArray->unbind();
}
}