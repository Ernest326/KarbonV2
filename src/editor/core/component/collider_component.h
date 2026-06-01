#pragma once
#include "component_inspector.h"
#include "scene/components/collider_component.h"

namespace Karbon {
class ColliderComponentInspector : public ComponentInspector {
public:
    const char* GetName() const override { return "Collider 3D"; }
    bool CanInspect(entt::registry& registry, entt::entity entity) const override {
        return registry.all_of<ColliderComponent>(entity);
    }
    void Inspect(entt::registry& registry, entt::entity entity) override {
        auto& collider = registry.get<ColliderComponent>(entity);
        const char* colliderTypes[] = { "Box", "Sphere", "Capsule", "Mesh", "Plane" };
        if (ImGui::Combo("Type", reinterpret_cast<int*>(&collider.type), colliderTypes, IM_ARRAYSIZE(colliderTypes))) {
            // No additional logic needed here for now, but this is where you'd handle changes to the Collider type in the physics system
        }
        if (collider.type == ColliderComponent::Type::Box) {
            ImGui::DragFloat3("Half Extents", &collider.halfExtents.x, 0.1f);
        } else if (collider.type == ColliderComponent::Type::Sphere || collider.type == ColliderComponent::Type::Capsule) {
            ImGui::DragFloat("Radius", &collider.radius, 0.1f);
        }
    }
};
} // namespace Karbon