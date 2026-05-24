#pragma once
#include <glm/glm.hpp>

namespace Karbon {
struct PointLightComponent {
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float radius = 10.0f;
    float falloff = 2.0f;
    bool castShadows = false;
    bool dirty = true;
};

} // namespace Karbon