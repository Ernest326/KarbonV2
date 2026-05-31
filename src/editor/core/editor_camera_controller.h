#pragma once
#include "graphics/camera.h"
#include "core/window.h"
#include <glm/glm.hpp>

namespace Karbon {

class EditorCameraController {
public:
    EditorCameraController(Camera* camera, Window* window);
    
    void OnUpdate(float deltaTime);
    void SetViewportActive(bool active) { m_viewportActive = active; }
    bool IsCapturingMouse() const { return m_capturing; }
    void Release(); // force release (ESC, focus loss, etc.)

private:
    void UpdateMouseLook();
    void UpdateMovement(float deltaTime);

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