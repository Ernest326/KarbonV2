#include "camera.h"
#include "../core/application.h"

namespace Karbon {

Camera::Camera(const glm::vec3& position, const glm::vec3& rotation, float fov, float nearPlane, float farPlane) {
    this->position = position;
    this->rotation = rotation;
    m_fov = fov;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_aspectRatio = Application::Get().getWindow().getAspectRatio(); 
    setProjection(fov, nearPlane, farPlane);
    updateViewMatrix();
}

void Camera::setPosition(const glm::vec3& position) {
    this->position = position;
    updateViewMatrix();
}

void Camera::setRotation(const glm::vec3& rotation) {
    this->rotation = rotation;
    updateViewMatrix();
}

void Camera::setProjection(float fov, float nearPlane, float farPlane) {
    projectionMatrix = glm::perspective(glm::radians(fov), m_aspectRatio, nearPlane, farPlane);
}

void Camera::setProjection(float fov, float nearPlane, float farPlane, float aspectRatio) {
    projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

void Camera::setAspectRatio(float aspectRatio) {
    m_aspectRatio = aspectRatio;
    setProjection(m_fov, m_nearPlane, m_farPlane, aspectRatio);
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
    glm::vec3 front = forward();
    viewMatrix = glm::lookAt(position, position + front, glm::vec3(0.0f, 1.0f, 0.0f));
}
}

