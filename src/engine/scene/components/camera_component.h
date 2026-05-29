#pragma once

#include "../../graphics/camera.h"

namespace Karbon {

struct CameraComponent {
    Camera camera;
    bool primary = false;

    CameraComponent() = default;
    explicit CameraComponent(const Camera& camera, bool primary = false)
        : camera(camera), primary(primary) {
    }
};

}