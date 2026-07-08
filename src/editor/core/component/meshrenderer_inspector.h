#pragma once
#include "component_inspector.h"
#include "scene/components/meshrenderer_component.h"

namespace Karbon {

class MeshRendererComponentInspector : public ComponentInspector {
public:
    const char* getName() const override { return "Mesh Renderer"; }

    bool canInspect(entt::registry& registry, entt::entity entity) const override {
        return registry.all_of<MeshRendererComponent>(entity);
    }

    void inspect(entt::registry& registry, entt::entity entity) override {
        auto& meshRenderer = registry.get<MeshRendererComponent>(entity);

        //Material handle
        ImGui::InputInt("Material Handle", reinterpret_cast<int*>(&meshRenderer.material));
        ImGui::Checkbox("Visible", &meshRenderer.visible);
        ImGui::Checkbox("Cast Shadows", &meshRenderer.castShadows);
        ImGui::Checkbox("Receive Shadows", &meshRenderer.receiveShadows);
    }
};
}