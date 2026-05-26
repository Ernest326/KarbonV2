#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../graphics/buffers/buffers.h"

namespace Karbon {
class Cube {
private:
    static std::unique_ptr<VBO> vertexBuffer;
    static std::unique_ptr<IBO> indexBuffer;
    static std::unique_ptr<VBO> texCoordBuffer;
    static std::unique_ptr<VBO> normalBuffer;
    static std::unique_ptr<VAO> vertexArray;
public:
    Cube(glm::vec3 position = glm::vec3(0.0f), glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f));
    glm::mat4 getModelMatrix() const;

    glm::vec3 getPosition() const { return position; }
    const glm::quat& getRotation() const { return rotation; }
    glm::vec3 getScale() const { return scale; }

    void setPosition(const glm::vec3& position) { this->position = position; }
    void setRotation(const glm::quat& rotation) { this->rotation = rotation; }
    void setScale(const glm::vec3& scale) { this->scale = scale; }

    VBO &getVertexBuffer() const { return *vertexBuffer; }
    IBO &getIndexBuffer() const { return *indexBuffer; }

    void draw() const;
private:
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};
}