#include "editor_panels.h"

#include <imgui.h>
#include <cmath>
#include "../component/transform_component.h"
#include "../component/pointlight_component.h"
#include "../component/rigidbody_component.h"
#include "../component/collider_component.h"
#include "../component/meshrenderer_component.h"
#include "../component/directional_light_component.h"
#include "../component/spotlight_component.h"
#include <glm/gtc/quaternion.hpp>
#include "scene/components/hierarchy_component.h"
#include "scene/components/id_component.h"
#include "scene/components/transform.h"
#include "scene/scene.h"

namespace Karbon {

namespace {

void DrawHierarchyNode(Scene& scene, entt::entity entity, entt::entity* selectedEntity) {
    auto& registry = scene.getRegistry();
    if (!registry.valid(entity)) {
        return;
    }

    auto& tag = registry.get<TagComponent>(entity);
    auto& hierarchy = registry.get<HierarchyComponent>(entity);

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
    if (selectedEntity && *selectedEntity == entity) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (hierarchy.children.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    ImGui::PushID(static_cast<int>(entt::to_integral(entity)));
    bool opened = ImGui::TreeNodeEx(tag.tag.c_str(), flags);
    if (ImGui::IsItemClicked()) {
        if (selectedEntity) {
            *selectedEntity = entity;
        }
    }

    if (opened && !hierarchy.children.empty()) {
        for (auto child : hierarchy.children) {
            DrawHierarchyNode(scene, child, selectedEntity);
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

} // namespace

InspectorPanel::InspectorPanel() {
    m_componentInspectors.push_back(std::make_unique<TransformComponentInspector>());
    m_componentInspectors.push_back(std::make_unique<PointLightComponentInspector>());
    m_componentInspectors.push_back(std::make_unique<RigidbodyComponentInspector>());
    m_componentInspectors.push_back(std::make_unique<ColliderComponentInspector>());
    m_componentInspectors.push_back(std::make_unique<MeshRendererComponentInspector>());
    m_componentInspectors.push_back(std::make_unique<DirectionalLightComponentInspector>());
    m_componentInspectors.push_back(std::make_unique<SpotLightComponentInspector>());
}

void HierarchyPanel::Draw(Scene* scene, entt::entity* selectedEntity) {
    if (!m_open) {
        return;
    }

    ImGui::Begin("Hierarchy", &m_open);

    if (scene) {
        ImGui::Text("Scene Entities");
        ImGui::Separator();

        auto& registry = scene->getRegistry();
        auto view = registry.view<HierarchyComponent, TagComponent>();
        view.each([&](auto entity, HierarchyComponent& hierarchy, TagComponent&) {
            if (hierarchy.parent == entt::null) {
                DrawHierarchyNode(*scene, entity, selectedEntity);
            }
        });
    } else {
        ImGui::Text("No scene loaded");
    }

    ImGui::End();
}

void InspectorPanel::Draw(Scene* scene, entt::entity* selectedEntity) {
    if (!m_open) {
        return;
    }

    ImGui::Begin("Inspector", &m_open);

    bool hasSelection = false;
    entt::entity entity = entt::null;
    if (scene && selectedEntity) {
        auto& registry = scene->getRegistry();
        if (registry.valid(*selectedEntity)) {
            entity = *selectedEntity;
            hasSelection = true;
        }
    }

    if (!hasSelection) {
        ImGui::Text("No entity selected");
    } else {
        auto& registry = scene->getRegistry();

        if (registry.all_of<TagComponent>(entity)) {
            auto& tag = registry.get<TagComponent>(entity);
            ImGui::Text("%s", tag.tag.c_str());
        }

        for (const auto& inspector : m_componentInspectors) {
            if (!inspector->CanInspect(registry, entity)) {
                continue;
            }

            if (ImGui::CollapsingHeader(inspector->GetName(), ImGuiTreeNodeFlags_DefaultOpen)) {
                inspector->Inspect(registry, entity);
            }
        }
    }

    ImGui::BeginDisabled(!hasSelection);
    if (ImGui::Button("Add Component")) {
        ImGui::OpenPopup("AddComponentPopup");
    }
    ImGui::EndDisabled();

    if (ImGui::BeginPopup("AddComponentPopup")) {
        if (ImGui::MenuItem("Transform")) {}
        if (ImGui::MenuItem("Mesh Renderer")) {}
        if (ImGui::MenuItem("Rigidbody")) {}
        if (ImGui::MenuItem("Point Light")) {}
        ImGui::EndPopup();
    }

    ImGui::Separator();
    ImGui::End();
}

void ContentBrowserPanel::Draw(Scene* /*scene*/) {
    if (!m_open) {
        return;
    }

    ImGui::Begin("Content Browser", &m_open);

    ImGui::Text("assets > meshes");
    ImGui::Separator();

    float cellSize = 80.0f;
    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = static_cast<int>(panelWidth / cellSize);
    if (columnCount < 1) {
        columnCount = 1;
    }

    ImGui::Columns(columnCount, nullptr, false);

    for (int i = 0; i < 12; i++) {
        ImGui::Button("file", ImVec2(cellSize - 10, cellSize - 10));
        ImGui::Text("asset_%d.fbx", i);
        ImGui::NextColumn();
    }

    ImGui::Columns(1);
    ImGui::End();
}

void StatsPanel::Draw(Scene* /*scene*/) {
    if (!m_open) {
        return;
    }

    ImGui::Begin("Stats", &m_open);

    float frameRate = ImGui::GetIO().Framerate;
    float frameTimeMs = frameRate > 0.0f ? (1000.0f / frameRate) : 0.0f;
    ImGui::Text("FPS: %.1f", frameRate);
    ImGui::Text("Frame Time: %.3f ms", frameTimeMs);
    ImGui::Separator();
    ImGui::Text("Draw Calls: 0");
    ImGui::Text("Triangles: 0");
    ImGui::Text("Entities: 0");
    ImGui::Separator();

    ImGui::End();
}

} // namespace Karbon
