#pragma once
#include <functional>
#include <memory>
#include <imgui.h>
#include "graphics/framebuffer.h"

namespace Karbon {

class Scene;

class EditorViewportPanel {
public:
    void initialize();
    void draw(Scene* scene, const std::function<void()>& onDrawGizmos=std::function<void()>());

    bool isFocused() const { return m_focused; }
    bool isHovered() const { return m_hovered; }
    bool isActive() const { return m_focused || m_hovered; }
    ImVec2 getSize() const { return m_size; }
    Framebuffer* getFramebuffer() const { return m_framebuffer.get(); }

    bool consumeClick(int& outX, int& outY);
    bool m_clicked = false;
    int m_clickX = 0;
    int m_clickY = 0;

    // Converts a whole-window pixel position (e.g. from InputSystem::getMousePosition())
    // into a framebuffer-local pixel position (bottom-left origin, matching glReadPixels).
    // Returns false if outside the viewport image or before the first draw() call.
    bool windowToFramebuffer(double windowX, double windowY, int& outX, int& outY) const;

private:
    void resizeIfNeeded(const ImVec2& size, Scene* scene);
    void updateCameraAspect(Scene* scene, const ImVec2& size);

    ImVec2 m_size = ImVec2(0.0f, 0.0f);
    ImVec2 m_imagePos = ImVec2(0.0f, 0.0f);
    bool m_focused = false;
    bool m_hovered = false;
    std::unique_ptr<Framebuffer> m_framebuffer;
};

} // namespace Karbon
