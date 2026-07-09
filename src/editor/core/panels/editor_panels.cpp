#include "editor_panels.h"

#include <imgui.h>
#include <cmath>
#include "core/component/transform_inspector.h"
#include "core/component/pointlight_inspector.h"
#include "core/component/rigidbody_inspector.h"
#include "core/component/collider_inspector.h"
#include "core/component/meshrenderer_inspector.h"
#include "core/component/directional_light_inspector.h"
#include "core/component/spotlight_inspector.h"
#include <glm/gtc/quaternion.hpp>
#include "scene/components/hierarchy_component.h"
#include "scene/components/id_component.h"
#include "scene/components/transform.h"
#include "scene/scene.h"
#include <string>
#include <vector>

namespace Karbon {

namespace {

constexpr const char* kEntityDragDropID = "HIERARCHY_ENTITY";

// Is `candidate` equal to or nested somewhere under `ancestor`? Used to block
// drag-drop reparenting that would create a cycle (a node dropped onto its
// own descendant).
bool isDescendant(entt::registry& registry, entt::entity ancestor, entt::entity candidate) {
    if (ancestor == candidate) return true;
    if (!registry.valid(ancestor) || !registry.all_of<HierarchyComponent>(ancestor)) return false;
    for (auto child : registry.get<HierarchyComponent>(ancestor).children) {
        if (isDescendant(registry, child, candidate)) return true;
    }
    return false;
}

void DrawHierarchyNode(Scene& scene, entt::entity entity, entt::entity* selectedEntity) {
    auto& registry = scene.getRegistry();
    if (!registry.valid(entity)) {
        return;
    }

    // Copied by value rather than kept as a reference: the context-menu actions
    // below (create/reparent/destroy) can grow or shrink entt's component pools,
    // which invalidates any reference held across those calls.
    std::string tagText = registry.get<TagComponent>(entity).tag;
    bool isLeaf = registry.get<HierarchyComponent>(entity).children.empty();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
    if (selectedEntity && *selectedEntity == entity) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (isLeaf) {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    ImGui::PushID(static_cast<int>(entt::to_integral(entity)));
    bool opened = ImGui::TreeNodeEx(tagText.c_str(), flags);
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        if (selectedEntity) {
            *selectedEntity = entity;
        }
    }

    // Drag source: carry this entity's handle so another row can accept it
    if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload(kEntityDragDropID, &entity, sizeof(entt::entity));
        ImGui::Text("%s", tagText.c_str());
        ImGui::EndDragDropSource();
    }

    // Drop target: dropping another entity here reparents it under this one
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kEntityDragDropID)) {
            entt::entity dragged = *static_cast<const entt::entity*>(payload->Data);
            if (dragged != entity && !isDescendant(registry, dragged, entity)) {
                scene.setParent(dragged, entity);
            }
        }
        ImGui::EndDragDropTarget();
    }

    bool destroyed = false;
    if (ImGui::BeginPopupContextItem()) {
        if (selectedEntity) {
            *selectedEntity = entity;
        }
        if (ImGui::MenuItem("Create Empty Child")) {
            entt::entity child = scene.createEntity("Entity");
            scene.setParent(child, entity);
        }
        bool hasParent = registry.get<HierarchyComponent>(entity).parent != entt::null;
        if (hasParent && ImGui::MenuItem("Unparent")) {
            scene.unparent(entity);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Delete Entity")) {
            if (selectedEntity && *selectedEntity == entity) {
                *selectedEntity = entt::null;
            }
            scene.destroyEntity(entity);
            destroyed = true;
        }
        ImGui::EndPopup();
    }

    if (!destroyed && opened && !isLeaf) {
        // Copy: recursing into children may itself create/destroy/reparent
        // entities from their own context menus, which can invalidate a live
        // reference/iterator into this entity's HierarchyComponent.
        std::vector<entt::entity> children = registry.get<HierarchyComponent>(entity).children;
        for (auto child : children) {
            DrawHierarchyNode(scene, child, selectedEntity);
        }
    }
    if (opened && !isLeaf) {
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

void HierarchyPanel::draw(Scene* scene, entt::entity* selectedEntity) {
    if (!m_open) {
        return;
    }

    ImGui::Begin("Hierarchy", &m_open);

    if (scene) {
        ImGui::Text("Scene Entities");
        ImGui::Separator();

        auto& registry = scene->getRegistry();

        // Snapshot root entities before recursing: a node's own context menu
        // (create/destroy/reparent) mutates the registry mid-draw, which is
        // unsafe to do while a live view/iterator over it is still in scope.
        std::vector<entt::entity> rootEntities;
        for (auto entity : registry.view<HierarchyComponent, TagComponent>()) {
            if (registry.get<HierarchyComponent>(entity).parent == entt::null) {
                rootEntities.push_back(entity);
            }
        }
        for (auto entity : rootEntities) {
            DrawHierarchyNode(*scene, entity, selectedEntity);
        }

        // Empty space below the tree: drop here to move an entity to root level
        ImGui::Dummy(ImGui::GetContentRegionAvail());
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kEntityDragDropID)) {
                entt::entity dragged = *static_cast<const entt::entity*>(payload->Data);
                scene->unparent(dragged);
            }
            ImGui::EndDragDropTarget();
        }

        // Right-click empty space: create a new root-level entity
        if (ImGui::BeginPopupContextWindow("HierarchyBackgroundContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem("Create Empty")) {
                scene->createEntity("Entity");
            }
            ImGui::EndPopup();
        }
    } else {
        ImGui::Text("No scene loaded");
    }

    ImGui::End();
}

void InspectorPanel::draw(Scene* scene, entt::entity* selectedEntity) {
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
            if (!inspector->canInspect(registry, entity)) {
                continue;
            }

            if (ImGui::CollapsingHeader(inspector->getName(), ImGuiTreeNodeFlags_DefaultOpen)) {
                inspector->inspect(registry, entity);
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

void ContentBrowserPanel::draw(Scene* /*scene*/) {
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

void StatsPanel::draw(Scene* /*scene*/) {
    if (!m_open) {
        return;
    }

    ImGui::Begin("Stats", &m_open);

    float frameRate = ImGui::GetIO().Framerate;
    float frameTimeMs = frameRate > 0.0f ? (1000.0f / frameRate) : 0.0f;
    ImGui::Text("FPS: %.1f", frameRate);
    ImGui::Text("Frame Time: %.3f ms", frameTimeMs);
    ImGui::Separator();
    ImGui::Text("draw Calls: 0");
    ImGui::Text("Triangles: 0");
    ImGui::Text("Entities: 0");
    ImGui::Separator();

    ImGui::End();
}

} // namespace Karbon
