#pragma once
#include <entt/entt.hpp>
#include <vector>
#include <imgui/imgui.h>
#include <glm/glm.hpp>

namespace Karbon {
class ComponentInspector {
public:
    virtual ~ComponentInspector() = default;
    virtual const char* GetName() const = 0;
    virtual bool CanInspect(entt::registry& registry, entt::entity entity) const = 0;
    virtual void Inspect(entt::registry& registry, entt::entity entity) = 0;
};
}