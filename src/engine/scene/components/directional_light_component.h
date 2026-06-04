#pragma once
#include <glm/glm.hpp>

namespace Karbon {

class DirectionalLightComponent {
public:
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    bool castShadows = false;
    bool dirty = true;
};

}