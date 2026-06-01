#include "editor_panels.h"

#include <imgui.h>
#include <cmath>
#include <glm/gtc/quaternion.hpp>
#include "scene/components/hierarchy_component.h"
#include "scene/components/id_component.h"
#include "scene/components/transform.h"
#include "scene/scene.h"

namespace Karbon {

namespace {

glm::quat NormalizeQuat(const glm::quat& rotation) {
    float len = std::sqrt(rotation.w * rotation.w + rotation.x * rotation.x + rotation.y * rotation.y + rotation.z * rotation.z);
    if (len <= 0.0f) {
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }
    float inv = 1.0f / len;
    return glm::quat(rotation.w * inv, rotation.x * inv, rotation.y * inv, rotation.z * inv);
}

glm::vec3 QuaternionToEulerDegrees(const glm::quat& rotation) {
    glm::quat q = NormalizeQuat(rotation);

    float sinr_cosp = 2.0f * (q.w * q.x + q.y * q.z);
    float cosr_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    float x = std::atan2(sinr_cosp, cosr_cosp);

    float sinp = 2.0f * (q.w * q.y - q.z * q.x);
    sinp = glm::clamp(sinp, -1.0f, 1.0f);
    float y = std::asin(sinp);

    float siny_cosp = 2.0f * (q.w * q.z + q.x * q.y);
    float cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    float z = std::atan2(siny_cosp, cosy_cosp);

    return glm::degrees(glm::vec3(x, y, z));
}

glm::quat EulerDegreesToQuaternion(const glm::vec3& eulerDegrees) {
    glm::vec3 radians = glm::radians(eulerDegrees);

    float cx = std::cos(radians.x * 0.5f);
    float sx = std::sin(radians.x * 0.5f);
    float cy = std::cos(radians.y * 0.5f);
    float sy = std::sin(radians.y * 0.5f);
    float cz = std::cos(radians.z * 0.5f);
    float sz = std::sin(radians.z * 0.5f);

    glm::quat q;
    q.w = cx * cy * cz + sx * sy * sz;
    q.x = sx * cy * cz - cx * sy * sz;
    q.y = cx * sy * cz + sx * cy * sz;
    q.z = cx * cy * sz - sx * sy * cz;
    return NormalizeQuat(q);
}

class TransformComponentInspector : public ComponentInspector {
public:
    const char* GetName() const override { return "Transform"; }

    bool CanInspect(entt::registry& registry, entt::entity entity) const override {
        return registry.all_of<TransformComponent>(entity);
    }

    void Inspect(entt::registry& registry, entt::entity entity) override {
        auto& transform = registry.get<TransformComponent>(entity);

        if (ImGui::DragFloat3("Position", &transform.position.x, 0.1f)) {
            transform.clearLocalMatrix();
        }

        glm::vec3 rotationEuler = QuaternionToEulerDegrees(transform.rotation);
        if (ImGui::DragFloat3("Rotation", &rotationEuler.x, 0.5f)) {
            transform.rotation = EulerDegreesToQuaternion(rotationEuler);
            transform.clearLocalMatrix();
        }

        if (ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f)) {
            transform.clearLocalMatrix();
        }
    }
};

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
