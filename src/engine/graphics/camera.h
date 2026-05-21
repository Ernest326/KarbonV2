#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Karbon {
class Camera {
public:
    Camera(const glm::vec3& position=glm::vec3(0.0f), const glm::vec3& rotation=glm::vec3(0.0f), float fov=45.0f, float nearPlane=0.1f, float farPlane=100.0f) : position(position), rotation(rotation) {
        setProjection(fov, nearPlane, farPlane);
        updateViewMatrix();
    }
    ~Camera() {};

    void setPosition(const glm::vec3& position);
    void setRotation(const glm::vec3& rotation);
    void setProjection(float fov, float nearPlane, float farPlane);

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

private:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    void updateViewMatrix();
};
}