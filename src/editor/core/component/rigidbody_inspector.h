#pragma once
#include "component_inspector.h"
#include "scene/components/rigidbody_component.h"

namespace Karbon {
class RigidbodyComponentInspector : public ComponentInspector {
public:
    const char* getName() const override { return "Rigidbody 3D"; }

    bool canInspect(entt::registry& registry, entt::entity entity) const override {
        return registry.all_of<RigidbodyComponent>(entity);
    }

    void inspect(entt::registry& registry, entt::entity entity) override {
        auto& rigidbody = registry.get<RigidbodyComponent>(entity);

        if (ImGui::Combo("Type", reinterpret_cast<int*>(&rigidbody.type), "Static\0Kinematic\0Dynamic\0")) {
            // No additional logic needed here for now, but this is where you'd handle changes to the Rigidbody type in the physics system
        }
        if (ImGui::Combo("Physics Type", reinterpret_cast<int*>(&rigidbody.physicsType), "Discrete\0Continuous\0")) {
            // No additional logic needed here for now, but this is where you'd handle changes to the physics type in the physics system
        }
        if (ImGui::DragFloat("Mass", &rigidbody.mass, 0.1f, 0.0f)) {
            rigidbody.mass = std::max(rigidbody.mass, 0.0f);
        }
        if (ImGui::DragFloat("Friction", &rigidbody.friction, 0.01f, 0.0f, 1.0f)) {
            rigidbody.friction = std::clamp(rigidbody.friction, 0.0f, 1.0f);
        }
        if (ImGui::DragFloat("Restitution", &rigidbody.restitution, 0.01f, 0.0f, 1.0f)) {
            rigidbody.restitution = std::clamp(rigidbody.restitution, 0.0f, 1.0f);
        }
        ImGui::Checkbox("Is Trigger", &rigidbody.isTrigger);

    }
};
}