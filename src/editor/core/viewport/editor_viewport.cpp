#include "editor_viewport.h"

#include "scene/components/camera_component.h"
#include "scene/scene.h"

namespace Karbon {

void EditorViewportPanel::Initialize() {
    m_framebuffer = std::make_unique<Framebuffer>(1280, 720);
}

void EditorViewportPanel::Draw(Scene* scene, const std::function<void()>& onDrawGizmos) {
    if (!m_framebuffer) {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    m_focused = ImGui::IsWindowFocused();
    m_hovered = ImGui::IsWindowHovered();

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    ResizeIfNeeded(viewportPanelSize, scene);

    uint32_t textureID = m_framebuffer->getColorAttachment();
    ImGui::Image((void*)(uintptr_t)textureID, viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));

    if(onDrawGizmos) {
        onDrawGizmos();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void EditorViewportPanel::ResizeIfNeeded(const ImVec2& size, Scene* scene) {
    if (size.x <= 0.0f || size.y <= 0.0f) {
        return;
    }

    if (size.x == m_size.x && size.y == m_size.y) {
        return;
    }

    m_size = size;
    m_framebuffer->resize(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));
    UpdateCameraAspect(scene, size);
}

void EditorViewportPanel::UpdateCameraAspect(Scene* scene, const ImVec2& size) {
    if (!scene || size.y <= 0.0f) {
        return;
    }

    entt::entity cameraEntity = scene->getPrimaryCameraEntity();
    if (cameraEntity == entt::null) {
        return;
    }

    CameraComponent& cameraComponent = scene->getRegistry().get<CameraComponent>(cameraEntity);
    cameraComponent.camera.setAspectRatio(size.x / size.y);
}

} // namespace Karbon
