#include "camera.h"
#include "../core/application.h"

namespace Karbon {
void Camera::setPosition(const glm::vec3& position) {
    this->position = position;
    updateViewMatrix();
}

void Camera::setRotation(const glm::vec3& rotation) {
    this->rotation = rotation;
    updateViewMatrix();
}

void Camera::setProjection(float fov, float nearPlane, float farPlane) {
    projectionMatrix = glm::perspective(glm::radians(fov), Application::Get().getWindow().getAspectRatio(), nearPlane, farPlane);
}

const glm::vec3& Camera::getPosition() const {
    return position;
}

const glm::vec3& Camera::getRotation() const {
    return rotation;
}

const glm::mat4& Camera::getViewMatrix() const {
    return viewMatrix;
}

const glm::mat4& Camera::getProjectionMatrix() const {
    return projectionMatrix;
}

void Camera::updateViewMatrix() {
    // Use left-handed coordinate system: use glm::lookAtLH
    glm::vec3 front = forward();
    viewMatrix = glm::lookAtLH(position, position + front, glm::vec3(0.0f, 1.0f, 0.0f));
}
}

