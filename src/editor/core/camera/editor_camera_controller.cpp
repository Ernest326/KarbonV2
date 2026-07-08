#include "core/camera/editor_camera_controller.h"
#include "graphics/camera.h"
#include "core/window.h"
#include "input/inputsystem.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Karbon {

EditorCameraController::EditorCameraController(Camera* camera, Window* window)
    : m_camera(camera), m_window(window) {
    if (m_camera) {
        glm::vec3 rot = m_camera->getRotation();
        m_pitch = rot.x;
        m_yaw   = rot.y;
    }
}

void EditorCameraController::release() {
    if (!m_capturing) return;
    m_capturing = false;
    m_firstFrame = false;
    glfwSetInputMode(m_window->getGLWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void EditorCameraController::onUpdate(float deltaTime) {
    if (!m_camera || !m_window) return;

    GLFWwindow* win = m_window->getGLWindow();
    bool rmbDown = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    bool escDown = glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS;

    // ESC always wins -- release immediately
    if (escDown && m_capturing) {
        release();
        return;
    }

    // Start capturing
    if (rmbDown && !m_capturing && m_viewportActive) {
        m_capturing = true;
        m_firstFrame = true;
        glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // Consume any stale mouse delta accumulated while over UI panels.
        InputSystem::get().getMouseDelta();
        return;
    }

    // Stop capturing
    if (!rmbDown && m_capturing) {
        release();
        return;
    }

    // Only process look/movement when actively capturing
    if (m_capturing) {
        if (m_firstFrame) {
            InputSystem::get().getMouseDelta();
            m_firstFrame = false;
            return;
        }
        updateMouseLook();
        updateMovement(deltaTime);
    }
}

void EditorCameraController::updateMouseLook() {
    auto [dx, dy] = InputSystem::get().getMouseDelta();
    m_yaw   += static_cast<float>(dx) * m_mouseSensitivity;
    m_pitch -= static_cast<float>(dy) * m_mouseSensitivity;
    m_pitch  = glm::clamp(m_pitch, -89.0f, 89.0f);

    m_camera->setRotation(glm::vec3(m_pitch, m_yaw, 0.0f));
}

void EditorCameraController::updateMovement(float deltaTime) {
    InputSystem& input = InputSystem::get();

    float speed = input.isKeyPressed(Key::LeftShift) ? m_sprintSpeed : m_moveSpeed;
    float velocity = speed * deltaTime;

    glm::vec3 movement(0.0f);
    if (input.isKeyPressed(Key::W)) movement += m_camera->forward();
    if (input.isKeyPressed(Key::S)) movement -= m_camera->forward();
    if (input.isKeyPressed(Key::D)) movement += m_camera->right();
    if (input.isKeyPressed(Key::A)) movement -= m_camera->right();
    if (input.isKeyPressed(Key::E)) movement += glm::vec3(0.0f, 1.0f, 0.0f);
    if (input.isKeyPressed(Key::Q)) movement -= glm::vec3(0.0f, 1.0f, 0.0f);

    if (glm::length(movement) > 0.0f) {
        movement = glm::normalize(movement) * velocity;
        m_camera->setPosition(m_camera->getPosition() + movement);
    }
}

} // namespace Karbon
