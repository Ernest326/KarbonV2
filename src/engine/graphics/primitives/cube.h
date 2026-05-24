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
    Cube(glm::vec3 position, glm::quat rotation, glm::vec3 scale);
    glm::mat4 getModelMatrix() const;

    glm::vec3 getPosition() const { return position; }
    const glm::quat& getRotation() const { return rotation; }
    glm::vec3 getScale() const { return scale; }

    void setPosition(const glm::vec3& position) { this->position = position; }
    void setRotation(const glm::quat& rotation) { this->rotation = rotation; }
    void setScale(const glm::vec3& scale) { this->scale = scale; }

    void draw() const;
private:
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};
}