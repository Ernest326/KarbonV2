#pragma once
#include <glm/glm.hpp>
#include <Jolt/Physics/Body/BodyID.h>

namespace Karbon {

struct ColliderComponent {
    enum class Type { Box, Sphere, Capsule, Mesh } type;
    glm::vec3 halfExtents{0.5f};   // Box
    float radius = 0.5f;           // Sphere/Capsule
    glm::vec3 offset{0.0f};        // Local offset from transform
 };

}