#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Karbon {

// Decomposes an affine matrix into position/rotation/scale, discarding shear
// via Gram-Schmidt orthonormalization. Degenerate (zero-scale) axes fall back
// to the identity basis.
inline void decomposeNoShear(const glm::mat4& matrix, glm::vec3& position, glm::quat& rotation, glm::vec3& scale) {
    position = glm::vec3(matrix[3]);

    glm::vec3 col0 = glm::vec3(matrix[0]);
    glm::vec3 col1 = glm::vec3(matrix[1]);
    glm::vec3 col2 = glm::vec3(matrix[2]);

    scale.x = glm::length(col0);
    if (scale.x > 0.000001f) {
        col0 /= scale.x;
    } else {
        col0 = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    col1 -= col0 * glm::dot(col1, col0);
    scale.y = glm::length(col1);
    if (scale.y > 0.000001f) {
        col1 /= scale.y;
    } else {
        col1 = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    col2 -= col0 * glm::dot(col2, col0) + col1 * glm::dot(col2, col1);
    scale.z = glm::length(col2);
    if (scale.z > 0.000001f) {
        col2 /= scale.z;
    } else {
        col2 = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    glm::mat3 rotMat(col0, col1, col2);
    rotation = glm::normalize(glm::quat_cast(rotMat));
}

inline glm::vec3 quaternionToEulerDegrees(const glm::quat& rotation) {
    return glm::degrees(glm::eulerAngles(glm::normalize(rotation)));
}

inline glm::quat eulerDegreesToQuaternion(const glm::vec3& eulerDegrees) {
    return glm::normalize(glm::quat(glm::radians(eulerDegrees)));
}

}
