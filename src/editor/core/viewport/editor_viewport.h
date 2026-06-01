#pragma once
#include <functional>
#include <memory>
#include <imgui.h>
#include "graphics/framebuffer.h"

namespace Karbon {

class Scene;

class EditorViewportPanel {
public:
    void Initialize();
    void Draw(Scene* scene, const std::function<void()>& onDrawGizmos=std::function<void()>());

    bool IsFocused() const { return m_focused; }
    bool IsHovered() const { return m_hovered; }
    bool IsActive() const { return m_focused || m_hovered; }
    ImVec2 GetSize() const { return m_size; }
    Framebuffer* GetFramebuffer() const { return m_framebuffer.get(); }

private:
    void ResizeIfNeeded(const ImVec2& size, Scene* scene);
    void UpdateCameraAspect(Scene* scene, const ImVec2& size);

    ImVec2 m_size = ImVec2(0.0f, 0.0f);
    bool m_focused = false;
    bool m_hovered = false;
    std::unique_ptr<Framebuffer> m_framebuffer;
};

} // namespace Karbon
