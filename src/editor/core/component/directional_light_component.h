#pragma once
#include "component_inspector.h"
#include "scene/components/directional_light_component.h"

namespace Karbon {
class DirectionalLightComponentInspector : public ComponentInspector {
public:
    const char* GetName() const override { return "Directional Light"; }

    bool CanInspect(entt::registry& registry, entt::entity entity) const override {
        return registry.all_of<DirectionalLightComponent>(entity);
    }

    void Inspect(entt::registry& registry, entt::entity entity) override {
        auto& dirLight = registry.get<DirectionalLightComponent>(entity); 

        if(ImGui::ColorEdit3("Color", &dirLight.color.x)) {
            dirLight.color = glm::vec3(glm::clamp(dirLight.color.r, 0.0f, 1.0f),
                                    glm::clamp(dirLight.color.g, 0.0f, 1.0f),
                                    glm::clamp(dirLight.color.b, 0.0f, 1.0f));
        }
        if(ImGui::DragFloat("Intensity", &dirLight.intensity, 0.1f, 0.0f)) {
            dirLight.intensity = std::max(dirLight.intensity, 0.0f);
        }
        if (ImGui::Checkbox("Cast Shadows", &dirLight.castShadows)) {
            // No additional logic needed here for now, but this is where you'd handle enabling/disabling shadow casting in the renderer
        }
    }
};
}