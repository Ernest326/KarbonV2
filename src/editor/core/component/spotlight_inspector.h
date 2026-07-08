#pragma once
#include "component_inspector.h"
#include "scene/components/spotlight_component.h"

namespace Karbon {
class SpotLightComponentInspector : public ComponentInspector {
public:
    const char* getName() const override { return "Spotlight"; }

    bool canInspect(entt::registry& registry, entt::entity entity) const override {
        return registry.all_of<SpotLightComponent>(entity);
    }

    void inspect(entt::registry& registry, entt::entity entity) override {
        auto& spotlight = registry.get<SpotLightComponent>(entity); 

        if(ImGui::ColorEdit3("Color", &spotlight.color.x)) {
            spotlight.color = glm::vec3(glm::clamp(spotlight.color.r, 0.0f, 1.0f),
                                    glm::clamp(spotlight.color.g, 0.0f, 1.0f),
                                    glm::clamp(spotlight.color.b, 0.0f, 1.0f));
        }
        if(ImGui::DragFloat("Intensity", &spotlight.intensity, 0.1f, 0.0f)) {
            spotlight.intensity = std::max(spotlight.intensity, 0.0f);
        }
        if(ImGui::DragFloat("Radius", &spotlight.radius, 0.1f, 0.0f)) {
            spotlight.radius = std::max(spotlight.radius, 0.0f);
        }
        if(ImGui::DragFloat("Falloff", &spotlight.falloff, 0.1f, 0.1f)) {
            spotlight.falloff = std::max(spotlight.falloff, 0.1f);
        }
        if(ImGui::DragFloat("Cutoff", &spotlight.cutOff, 0.01f, 0.0f, 1.0f)) {
            spotlight.cutOff = glm::clamp(spotlight.cutOff, 0.0f, 1.0f);
        }
        if(ImGui::DragFloat("Outer Cutoff", &spotlight.outerCutOff, 0.01f, 0.0f, 1.0f)) {
            spotlight.outerCutOff = glm::clamp(spotlight.outerCutOff, 0.0f, 1.0f);
        }
        if (ImGui::Checkbox("Cast Shadows", &spotlight.castShadows)) {
            // No additional logic needed here for now, but this is where you'd handle enabling/disabling shadow casting in the renderer
        }
    }
};
}