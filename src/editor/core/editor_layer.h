#pragma once

#include <entt/entt.hpp>
#include <imgui.h>
#include "core/layer.h"
#include "scene/scene.h"
#include "graphics/framebuffer.h"
#include "scene/cube_mesh.h"
#include "scene/entity.h"
#include "editor_camera_controller.h"
#include "events/key_event.h"

namespace Karbon {

class EditorLayer : public Layer {
public:
    explicit EditorLayer(Scene* scene);

    void OnUpdate(float deltaTime) override;
    void onAttach() override;
    void onImGuiRender() override;
    void OnEvent(Event& e) override;

private:

    bool OnKeyPress(KeyPressEvent& e);

    void setupDockSpace();
    void drawMenuBar();

    void drawHierarchy();
    void drawInspector();
    void drawContentBrowser();
    void drawStats();
    void drawViewport();

    bool m_showHierarchy = true;
    bool m_showInspector = true;
    bool m_showContentBrowser = true;
    bool m_showStats = true;

    ImVec2 m_viewportSize = ImVec2(0, 0);
    bool m_viewportFocused = false;
    bool m_viewportHovered = false;

    Scene* m_scene = nullptr;
    entt::entity m_selectedEntity = entt::null;
    bool m_bootstrapped = false;
    bool m_styleInitialized = false;
    std::unique_ptr<Framebuffer> m_viewportFramebuffer;

    CubeMesh m_cubeMesh;
    Entity m_testCube;

    entt::entity m_editorCamera;
    std::unique_ptr<EditorCameraController> m_cameraController;

};

}