#include "editor_layer.h"

#include <imgui.h>
#include "scene/components/id_component.h"

namespace Karbon {

EditorLayer::EditorLayer(Scene* scene) : m_scene(scene) {
}

void EditorLayer::onAttach() {
    if (!m_scene || m_bootstrapped) {
        return;
    }

    entt::entity camera = m_scene->createEntity("Editor Camera");
    m_scene->createEntity("Directional Light");
    m_scene->createEntity("Cube");

    m_selectedEntity = camera;

    m_bootstrapped = true;
}

void EditorLayer::onImGuiRender() {
    ImGui::Begin("Editor");

    const ImGuiIO& io = ImGui::GetIO();
    const float fps = io.Framerate;
    const float frameMs = (fps > 0.0f) ? (1000.0f / fps) : 0.0f;
    ImGui::Text("Frame: %.3f ms (%.1f FPS)", frameMs, fps);

    if (!m_scene) {
        ImGui::Text("No scene loaded.");
        ImGui::End();
        return;
    }

    auto& registry = m_scene->getRegistry();
    auto view = registry.view<TagComponent, IDComponent>();

    ImGui::Separator();
    ImGui::Text("Entities (%d)", static_cast<int>(view.size_hint()));

    ImGui::BeginChild("EntityList", ImVec2(0.0f, 160.0f), true);
    for (auto entity : view) {
        const auto& tag = view.get<TagComponent>(entity).tag;
        const bool selected = (entity == m_selectedEntity);
        if (ImGui::Selectable(tag.c_str(), selected)) {
            m_selectedEntity = entity;
        }
    }
    ImGui::EndChild();

    ImGui::Separator();
    if (m_selectedEntity != entt::null && registry.valid(m_selectedEntity)) {
        const auto& tag = registry.get<TagComponent>(m_selectedEntity);
        const auto& id = registry.get<IDComponent>(m_selectedEntity);
        ImGui::Text("Selection");
        ImGui::Text("Tag: %s", tag.tag.c_str());
        ImGui::Text("ID: %llu", static_cast<unsigned long long>(id.id));
    } else {
        ImGui::Text("No entity selected.");
    }

    ImGui::End();
}

}