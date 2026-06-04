#pragma once
#include <glm/glm.hpp>

namespace Karbon {
class SpotLightComponent {
public:
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float radius = 10.0f;
    float falloff = 2.0f;
    float cutOff = glm::cos(glm::radians(12.5f));
    float outerCutOff = glm::cos(glm::radians(17.5f));
    bool castShadows = false;
    bool dirty = true;
};
}