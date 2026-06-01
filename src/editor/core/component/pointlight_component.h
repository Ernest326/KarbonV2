#pragma once
#include "component_inspector.h"
#include "scene/components/pointlight_component.h"

namespace Karbon {
class PointLightComponentInspector : public ComponentInspector {
public:
    const char* GetName() const override { return "Point Light"; }

    bool CanInspect(entt::registry& registry, entt::entity entity) const override {
        return registry.all_of<PointLightComponent>(entity);
    }

    void Inspect(entt::registry& registry, entt::entity entity) override {
        auto& pointLight = registry.get<PointLightComponent>(entity);
       
        if (ImGui::ColorEdit3("Color", &pointLight.color.x)) {
           pointLight.color = glm::vec3(glm::clamp(pointLight.color.r, 0.0f, 1.0f),
                                    glm::clamp(pointLight.color.g, 0.0f, 1.0f),
                                    glm::clamp(pointLight.color.b, 0.0f, 1.0f));
        }
        if (ImGui::DragFloat("Intensity", &pointLight.intensity, 0.1f, 0.0f)) {
            pointLight.intensity = std::max(pointLight.intensity, 0.0f);
        }
        if (ImGui::DragFloat("Radius", &pointLight.radius, 0.1f, 0.0f)) {
            pointLight.radius = std::max(pointLight.radius, 0.0f);
        }
        if (ImGui::DragFloat("Falloff", &pointLight.falloff, 0.1f, 0.1f)) {
            pointLight.falloff = std::max(pointLight.falloff, 0.1f);
        }
        if (ImGui::Checkbox("Cast Shadows", &pointLight.castShadows)) {
            // No additional logic needed here for now, but this is where you'd handle enabling/disabling shadow casting in the renderer
        }

    }
};
}