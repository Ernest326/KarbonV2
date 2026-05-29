#pragma once

#include "camera.h"
#include "../input/inputsystem.h"

namespace Karbon {

class SpectatorCamera {
public:
    SpectatorCamera(const glm::vec3& position = glm::vec3(0.0f),
                    const glm::vec3& rotation = glm::vec3(0.0f),
                    float fov = 45.0f,
                    float nearPlane = 0.1f,
                    float farPlane = 100.0f);

    SpectatorCamera(Camera* target,
                    const glm::vec3& position = glm::vec3(0.0f),
                    const glm::vec3& rotation = glm::vec3(0.0f),
                    float fov = 45.0f,
                    float nearPlane = 0.1f,
                    float farPlane = 100.0f);

    void update(float deltaTime);

    void setMovementSpeed(float speed);
    void setMouseSensitivity(float sensitivity);
    void setProjection(float fov, float nearPlane, float farPlane);
    void setPosition(const glm::vec3& position);
    void setRotation(const glm::vec3& rotation);

    const glm::vec3& getPosition() const;
    const glm::vec3& getRotation() const;
    const glm::mat4& getViewMatrix() const;
    const glm::mat4& getProjectionMatrix() const;

    Camera& getCamera() { return *m_camera; }

private:
    void syncCamera();

private:
    Camera m_ownedCamera;
    Camera* m_camera = nullptr;
    float m_moveSpeed;
    float m_mouseSensitivity;
    float m_fov;
    float m_nearPlane;
    float m_farPlane;
    float m_pitch;
    float m_yaw;
};

}