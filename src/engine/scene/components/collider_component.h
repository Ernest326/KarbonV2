#pragma once
#include <glm/glm.hpp>

namespace Karbon {

struct ColliderComponent {
    enum class Type { Box, Sphere, Capsule, Mesh, Plane } type;
    glm::vec3 halfExtents{0.5f};   // Box
    float radius = 0.5f;           // Sphere/Capsule
    glm::vec3 offset{0.0f};        // Local offset from transform
 };

}