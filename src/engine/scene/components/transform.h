#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Karbon {

struct TransformComponent {
    glm::vec3 position{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f};
    glm::mat4 getLocalMatrix() const {
        return glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
    }
};

struct WorldTransformComponent {
    glm::mat4 matrix{1.0f};
    glm::vec3 worldPosition{0.0f};
    glm::quat worldRotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 worldScale{1.0f};
};

}