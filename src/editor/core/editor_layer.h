#pragma once

#include <entt/entt.hpp>
#include "core/layer.h"
#include "events/key_event.h"
#include "scene/cube_mesh.h"
#include "scene/sphere_mesh.h"
#include "scene/entity.h"
#include "scene/scene.h"
#include "core/camera/editor_camera.h"
#include "core/panels/editor_panels.h"
#include "core/viewport/editor_viewport.h"
#include "graphics/grid.h"
#include "graphics/shader.h"
#include "scene/model.h"

namespace Karbon {

struct GizmoSettings {
    bool snap = false;
    float snapValue = 0.5f; // For translation and scale
    float snapAngle = 15.0f; // For rotation
    enum class Mode { Local, World } mode = Mode::Local;
    enum class GizmoType { None, Translate, Rotate, Scale } gizmoType = GizmoType::Translate;
};

class EditorLayer : public Layer {
public:
    explicit EditorLayer(Scene* scene);

    void onUpdate(float deltaTime) override;
    void onAttach() override;
    void onImGuiRender() override;
    void onEvent(Event& e) override;
    void onRender() override;

private:

    bool onKeyPress(KeyPressEvent& e);
    bool onKeyRelease(KeyReleaseEvent& e);
    bool gizmoControls(KeyPressEvent& e);

    void drawOutline(Mesh* mesh, const glm::mat4& worldMatrix, Camera* camera);
    
    std::unique_ptr<Framebuffer> m_pickingFramebuffer;
    std::unique_ptr<Shader> m_pickingShader;
    std::unique_ptr<Shader> m_outlineShader;
    glm::vec4 encodeEntityID(entt::entity e);
    entt::entity decodeEntity(const GLubyte* pixel);
    void doPickingPass(int x, int y);

    void setupDockSpace();
    void drawMenuBar();

    GizmoSettings m_gizmoSettings;
    void drawGizmos(Scene* scene);
    bool m_snapGizmo = false;

    Scene* m_scene = nullptr;
    entt::entity m_selectedEntity = entt::null;
    bool m_bootstrapped = false;
    bool m_styleInitialized = false;

    std::unique_ptr<Grid> m_grid; 

    EditorViewportPanel m_viewport;
    EditorCamera m_editorCamera;

    HierarchyPanel m_hierarchyPanel;
    InspectorPanel m_inspectorPanel;
    ContentBrowserPanel m_contentBrowserPanel;
    StatsPanel m_statsPanel;

    CubeMesh m_cubeMesh;
    SphereMesh m_sphereMesh;
    std::unique_ptr<Model> m_monkeModel;
    Entity m_testCube;

};

}