#pragma once
#include "graphics/camera.h"
#include "core/window.h"
#include <glm/glm.hpp>

namespace Karbon {

class EditorCameraController {
public:
    EditorCameraController(Camera* camera, Window* window);

    void onUpdate(float deltaTime);
    void setViewportActive(bool active) { m_viewportActive = active; }
    bool isCapturingMouse() const { return m_capturing; }
    void release(); // force release (ESC, focus loss, etc.)

private:
    void updateMouseLook();
    void updateMovement(float deltaTime);

    Camera* m_camera = nullptr;
    Window* m_window = nullptr;

    bool m_viewportActive = false;
    bool m_capturing = false;
    bool m_firstFrame = false; // swallow the stale delta on capture start

    float m_moveSpeed = 6.0f;
    float m_sprintSpeed = 15.0f;
    float m_mouseSensitivity = 0.12f;
    float m_pitch = 0.0f;
    float m_yaw = 0.0f;
};

} // namespace Karbon
