#pragma once
#include "component_inspector.h"
#include "scene/components/transform.h"
#include <glm/gtc/quaternion.hpp>

namespace Karbon {

// inline: this header is included from multiple translation units (ODR)
inline glm::vec3 QuaternionToEulerDegrees(const glm::quat& rotation) {
    return glm::degrees(glm::eulerAngles(glm::normalize(rotation)));
}

inline glm::quat EulerDegreesToQuaternion(const glm::vec3& eulerDegrees) {
    return glm::normalize(glm::quat(glm::radians(eulerDegrees)));
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