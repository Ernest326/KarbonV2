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

    // Returns true once if a right-press+release completed with only trivial
    // movement (i.e. a click, not a look-around drag). outX/outY are in the
    // same whole-window pixel space as InputSystem::getMousePosition().
    bool consumeRightClick(double& outX, double& outY);

private:
    void updateMouseLook();
    void updateMovement(float deltaTime);

    Camera* m_camera = nullptr;
    Window* m_window = nullptr;

    bool m_viewportActive = false;
    bool m_capturing = false;
    bool m_firstFrame = false; // swallow the stale delta on capture start

    // Distinguishes a plain right-click from a look-around drag: total unsigned
    // mouse movement accumulated since capture started. Below the threshold on
    // release, it's treated as a click instead of a drag.
    double m_capturedMovement = 0.0;
    static constexpr double kClickMovementThreshold = 6.0;
    bool m_rightClickReady = false;
    double m_rightClickX = 0.0;
    double m_rightClickY = 0.0;

    float m_moveSpeed = 6.0f;
    float m_sprintSpeed = 15.0f;
    float m_mouseSensitivity = 0.12f;
    float m_pitch = 0.0f;
    float m_yaw = 0.0f;
};

} // namespace Karbon
