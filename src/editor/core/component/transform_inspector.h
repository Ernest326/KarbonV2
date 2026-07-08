#pragma once
#include "component_inspector.h"
#include "scene/components/transform.h"
#include "utils/math_utils.h"

namespace Karbon {

class TransformComponentInspector : public ComponentInspector {
public:
    const char* getName() const override { return "Transform"; }

    bool canInspect(entt::registry& registry, entt::entity entity) const override {
        return registry.all_of<TransformComponent>(entity);
    }

    void inspect(entt::registry& registry, entt::entity entity) override {
        auto& transform = registry.get<TransformComponent>(entity);

        if (ImGui::DragFloat3("Position", &transform.position.x, 0.1f)) {
            transform.clearLocalMatrix();
        }

        glm::vec3 rotationEuler = quaternionToEulerDegrees(transform.rotation);
        if (ImGui::DragFloat3("Rotation", &rotationEuler.x, 0.5f)) {
            transform.rotation = eulerDegreesToQuaternion(rotationEuler);
            transform.clearLocalMatrix();
        }

        if (ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f)) {
            transform.clearLocalMatrix();
        }
    }
};
}