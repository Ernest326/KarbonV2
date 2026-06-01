#pragma once
#include "component_inspector.h"
#include "scene/components/transform.h"

namespace Karbon {

glm::quat NormalizeQuat(const glm::quat& rotation) {
    float len = std::sqrt(rotation.w * rotation.w + rotation.x * rotation.x + rotation.y * rotation.y + rotation.z * rotation.z);
    if (len <= 0.0f) {
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }
    float inv = 1.0f / len;
    return glm::quat(rotation.w * inv, rotation.x * inv, rotation.y * inv, rotation.z * inv);
}

glm::vec3 QuaternionToEulerDegrees(const glm::quat& rotation) {
    glm::quat q = NormalizeQuat(rotation);

    float sinr_cosp = 2.0f * (q.w * q.x + q.y * q.z);
    float cosr_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    float x = std::atan2(sinr_cosp, cosr_cosp);

    float sinp = 2.0f * (q.w * q.y - q.z * q.x);
    sinp = glm::clamp(sinp, -1.0f, 1.0f);
    float y = std::asin(sinp);

    float siny_cosp = 2.0f * (q.w * q.z + q.x * q.y);
    float cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    float z = std::atan2(siny_cosp, cosy_cosp);

    return glm::degrees(glm::vec3(x, y, z));
}

glm::quat EulerDegreesToQuaternion(const glm::vec3& eulerDegrees) {
    glm::vec3 radians = glm::radians(eulerDegrees);

    float cx = std::cos(radians.x * 0.5f);
    float sx = std::sin(radians.x * 0.5f);
    float cy = std::cos(radians.y * 0.5f);
    float sy = std::sin(radians.y * 0.5f);
    float cz = std::cos(radians.z * 0.5f);
    float sz = std::sin(radians.z * 0.5f);

    glm::quat q;
    q.w = cx * cy * cz + sx * sy * sz;
    q.x = sx * cy * cz - cx * sy * sz;
    q.y = cx * sy * cz + sx * cy * sz;
    q.z = cx * cy * sz - sx * sy * cz;
    return NormalizeQuat(q);
}

class TransformComponentInspector : public ComponentInspector {
public:
    const char* GetName() const override { return "Transform"; }

    bool CanInspect(entt::registry& registry, entt::entity entity) const override {
        return registry.all_of<TransformComponent>(entity);
    }

    void Inspect(entt::registry& registry, entt::entity entity) override {
        auto& transform = registry.get<TransformComponent>(entity);

        if (ImGui::DragFloat3("Position", &transform.position.x, 0.1f)) {
            transform.clearLocalMatrix();
        }

        glm::vec3 rotationEuler = QuaternionToEulerDegrees(transform.rotation);
        if (ImGui::DragFloat3("Rotation", &rotationEuler.x, 0.5f)) {
            transform.rotation = EulerDegreesToQuaternion(rotationEuler);
            transform.clearLocalMatrix();
        }

        if (ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f)) {
            transform.clearLocalMatrix();
        }
    }
};
}