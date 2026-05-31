#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Karbon {
class Camera {
public:
    Camera(const glm::vec3& position=glm::vec3(0.0f), const glm::vec3& rotation=glm::vec3(0.0f), float fov=45.0f, float nearPlane=0.1f, float farPlane=100.0f)
        : position(position), rotation(rotation), m_fov(fov), m_nearPlane(nearPlane), m_farPlane(farPlane) {
        setProjection(fov, nearPlane, farPlane);
        updateViewMatrix();
    }
    ~Camera() {};

    void setPosition(const glm::vec3& position);
    void setRotation(const glm::vec3& rotation);
    void setProjection(float fov, float nearPlane, float farPlane);
    void setProjection(float fov, float nearPlane, float farPlane, float aspectRatio);

    glm::vec3 forward() const {
        float pitch = glm::radians(rotation.x);
        float yaw = glm::radians(rotation.y);
        return glm::normalize(glm::vec3(
            sin(yaw) * cos(pitch),
            sin(pitch),
            -cos(yaw) * cos(pitch)
        ));
    }

    glm::vec3 right() const {
        return glm::normalize(glm::cross(forward(), glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    glm::vec3 up() const {
        return glm::normalize(glm::cross(right(), forward()));
    }

    const glm::vec3& getPosition() const;
    const glm::vec3& getRotation() const;
    const glm::mat4& getViewMatrix() const;
    const glm::mat4& getProjectionMatrix() const;
    float getFov() const { return m_fov; }
    float getNearPlane() const { return m_nearPlane; }
    float getFarPlane() const { return m_farPlane; }
    float getAspectRatio() const { return m_aspectRatio; }

private:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    float m_fov = 45.0f;
    float m_nearPlane = 0.1f;
    float m_farPlane = 100.0f;
    float m_aspectRatio = 1.0f;

    void updateViewMatrix();
};
}