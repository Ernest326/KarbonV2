#include "editor_viewport.h"

#include "scene/components/camera_component.h"
#include "scene/scene.h"

namespace Karbon {

void EditorViewportPanel::initialize() {
    m_framebuffer = std::make_unique<Framebuffer>(1280, 720);
}

void EditorViewportPanel::draw(Scene* scene, const std::function<void()>& onDrawGizmos) {
    if (!m_framebuffer) {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    m_focused = ImGui::IsWindowFocused();
    m_hovered = ImGui::IsWindowHovered();

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    resizeIfNeeded(viewportPanelSize, scene);

    uint32_t textureID = m_framebuffer->getColorAttachment();
    ImVec2 imagePos = ImGui::GetCursorScreenPos();
    m_imagePos = imagePos;
    ImGui::Image((void*)(uintptr_t)textureID, viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));

    //Click handling
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_hovered) {
        ImVec2 mousePos = ImGui::GetMousePos();
        m_clickX = static_cast<int>(mousePos.x - imagePos.x);
        m_clickY = static_cast<int>(viewportPanelSize.y - (mousePos.y - imagePos.y));
        m_clicked = true;
    }

    if(onDrawGizmos) {
        onDrawGizmos();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

bool EditorViewportPanel::windowToFramebuffer(double windowX, double windowY, int& outX, int& outY) const {
    if (m_size.x <= 0.0f || m_size.y <= 0.0f) {
        return false;
    }

    outX = static_cast<int>(windowX - m_imagePos.x);
    outY = static_cast<int>(m_size.y - (windowY - m_imagePos.y));
    return outX >= 0 && outY >= 0 && outX < static_cast<int>(m_size.x) && outY < static_cast<int>(m_size.y);
}

bool EditorViewportPanel::consumeClick(int& outX, int& outY) {
    if (m_clicked) {
        outX = m_clickX;
        outY = m_clickY;
        m_clicked = false; // Reset click state after consuming
        return true;
    }
    return false;
}

void EditorViewportPanel::resizeIfNeeded(const ImVec2& size, Scene* scene) {
    if (size.x <= 0.0f || size.y <= 0.0f) {
        return;
    }

    if (size.x == m_size.x && size.y == m_size.y) {
        return;
    }

    m_size = size;
    m_framebuffer->resize(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));
    updateCameraAspect(scene, size);
}

void EditorViewportPanel::updateCameraAspect(Scene* scene, const ImVec2& size) {
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
