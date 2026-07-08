#pragma once
#include <entt/entt.hpp>
#include <vector>
#include <imgui/imgui.h>
#include <glm/glm.hpp>

namespace Karbon {
class ComponentInspector {
public:
    virtual ~ComponentInspector() = default;
    virtual const char* getName() const = 0;
    virtual bool canInspect(entt::registry& registry, entt::entity entity) const = 0;
    virtual void inspect(entt::registry& registry, entt::entity entity) = 0;
    virtual void renderGizmo(entt::registry& registry, entt::entity entity) {} // Optional override for rendering gizmos in the viewport
};
}