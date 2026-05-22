#include "plane.h"
#include <glm/gtc/quaternion.hpp>

namespace Karbon {

std::unique_ptr<VBO> Plane::vertexBuffer = nullptr;
std::unique_ptr<IBO> Plane::indexBuffer = nullptr;
std::unique_ptr<VBO> Plane::texCoordBuffer = nullptr;
std::unique_ptr<VAO> Plane::vertexArray = nullptr;

Plane::Plane(glm::vec3 position, glm::quat rotation, glm::vec3 scale) : position(position), rotation(rotation), scale(scale) {
    
    static bool buffersInitialized = false;
    if (!buffersInitialized) {
        GLfloat* plane_verts = new GLfloat[18] {
            // Front face
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f
        };

        GLuint indices[6] = {
            0, 1, 2, 2, 3, 0
        };

        GLfloat* glTexCoords = new GLfloat[12] {
            // Front face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
        };

        buffersInitialized = true;
        this->vertexBuffer = std::make_unique<VBO>(plane_verts, 18*sizeof(GLfloat));
        this->indexBuffer = std::make_unique<IBO>(indices, 6);
        this->texCoordBuffer = std::make_unique<VBO>(glTexCoords, 12*sizeof(GLfloat));
        this->vertexArray = std::make_unique<VAO>();
        this->vertexArray->addBuffer(*this->vertexBuffer, 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
        this->vertexArray->addBuffer(*this->texCoordBuffer, 1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);
        delete[] plane_verts;
        delete[] glTexCoords;
    }    
}

glm::mat4 Plane::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model *= glm::mat4_cast(rotation);
    model = glm::scale(model, scale);
    return model;
}

void Plane::draw() const {
    vertexArray->bind();
    indexBuffer->bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    indexBuffer->unbind();
    vertexArray->unbind();
}
}