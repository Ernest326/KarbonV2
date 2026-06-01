#pragma once

#include <entt/entt.hpp>
#include "core/layer.h"
#include "events/key_event.h"
#include "scene/cube_mesh.h"
#include "scene/entity.h"
#include "scene/scene.h"
#include "core/camera/editor_camera.h"
#include "core/panels/editor_panels.h"
#include "core/viewport/editor_viewport.h"

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

    Scene* m_scene = nullptr;
    entt::entity m_selectedEntity = entt::null;
    bool m_bootstrapped = false;
    bool m_styleInitialized = false;

    EditorViewportPanel m_viewport;
    EditorCamera m_editorCamera;

    HierarchyPanel m_hierarchyPanel;
    InspectorPanel m_inspectorPanel;
    ContentBrowserPanel m_contentBrowserPanel;
    StatsPanel m_statsPanel;

    CubeMesh m_cubeMesh;
    Entity m_testCube;

};

}