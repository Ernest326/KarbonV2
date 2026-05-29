#include "spectator_camera.h"

#include <algorithm>
#include <GLFW/glfw3.h>
#include <glm/common.hpp>
#include "../core/application.h"

namespace Karbon {

SpectatorCamera::SpectatorCamera(const glm::vec3& position,
                                 const glm::vec3& rotation,
                                 float fov,
                                 float nearPlane,
                                 float farPlane)
        : SpectatorCamera(nullptr, position, rotation, fov, nearPlane, farPlane) {
}

SpectatorCamera::SpectatorCamera(Camera* target,
                                                                 const glm::vec3& position,
                                                                 const glm::vec3& rotation,
                                                                 float fov,
                                                                 float nearPlane,
                                                                 float farPlane)
        : m_ownedCamera(position, rotation, fov, nearPlane, farPlane),
            m_camera(target ? target : &m_ownedCamera),
            m_moveSpeed(6.0f),
            m_mouseSensitivity(0.12f),
            m_fov(fov),
            m_nearPlane(nearPlane),
            m_farPlane(farPlane),
            m_pitch(rotation.x),
            m_yaw(rotation.y) {
        if (target) {
                m_camera->setPosition(position);
                m_camera->setRotation(rotation);
                m_camera->setProjection(fov, nearPlane, farPlane);
        }
    glfwSetInputMode(Application::Get().getWindow().getGLWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void SpectatorCamera::update(float deltaTime) {
    InputSystem& input = InputSystem::Get();

    m_camera->setProjection(m_fov, m_nearPlane, m_farPlane);

    glm::vec3 movement(0.0f);
    if (input.isKeyPressed(Key::W)) movement += m_camera->forward();
    if (input.isKeyPressed(Key::S)) movement -= m_camera->forward();
    if (input.isKeyPressed(Key::D)) movement += m_camera->right();
    if (input.isKeyPressed(Key::A)) movement -= m_camera->right();
    if (input.isKeyPressed(Key::Space)) movement += glm::vec3(0.0f, 1.0f, 0.0f);
    if (input.isKeyPressed(Key::LeftControl)) movement -= glm::vec3(0.0f, 1.0f, 0.0f);

    if (glm::length(movement) > 0.0f) {
        movement = glm::normalize(movement) * (m_moveSpeed * deltaTime);
        m_camera->setPosition(m_camera->getPosition() + movement);
    }

    const auto [mouseDeltaX, mouseDeltaY] = input.getMouseDelta();
    m_yaw += static_cast<float>(mouseDeltaX) * m_mouseSensitivity;
    m_pitch -= static_cast<float>(mouseDeltaY) * m_mouseSensitivity;

    syncCamera();
}

void SpectatorCamera::setMovementSpeed(float speed) {
    m_moveSpeed = speed;
}

void SpectatorCamera::setMouseSensitivity(float sensitivity) {
    m_mouseSensitivity = sensitivity;
}

void SpectatorCamera::setProjection(float fov, float nearPlane, float farPlane) {
    m_fov = fov;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_camera->setProjection(m_fov, m_nearPlane, m_farPlane);
}

void SpectatorCamera::setPosition(const glm::vec3& position) {
    m_camera->setPosition(position);
}

void SpectatorCamera::setRotation(const glm::vec3& rotation) {
    m_pitch = rotation.x;
    m_yaw = rotation.y;
    syncCamera();
}

const glm::vec3& SpectatorCamera::getPosition() const {
    return m_camera->getPosition();
}

const glm::vec3& SpectatorCamera::getRotation() const {
    return m_camera->getRotation();
}

const glm::mat4& SpectatorCamera::getViewMatrix() const {
    return m_camera->getViewMatrix();
}

const glm::mat4& SpectatorCamera::getProjectionMatrix() const {
    return m_camera->getProjectionMatrix();
}

void SpectatorCamera::syncCamera() {
    m_camera->setRotation(glm::vec3(m_pitch, m_yaw, 0.0f));
}

}