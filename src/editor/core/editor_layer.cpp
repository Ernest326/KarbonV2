#include "editor_layer.h"
#include "core/base.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include "core/application.h"
#include "scene/components/meshrenderer_component.h"
#include "scene/components/pointlight_component.h"

namespace Karbon {

EditorLayer::EditorLayer(Scene* scene) : m_scene(scene) {
}

void EditorLayer::onAttach() {
    //Some basic entity setup for testing
    if (!m_scene || m_bootstrapped) {
        return;
    }

    m_viewport.Initialize();
    Application::Get().setViewportFramebuffer(m_viewport.GetFramebuffer());

    m_editorCamera.Initialize(m_scene);

    // Test entities added in
    entt::entity directionalLight = m_scene->createEntity("Point Light");
    m_scene->getRegistry().get<TransformComponent>(directionalLight).position = glm::vec3(0.0f, 4.0f, 0.0f);
    m_scene->getRegistry().emplace<PointLightComponent>(directionalLight);

    entt::entity cube_id = m_scene->createEntity("Test Cube");
    m_testCube = Entity(cube_id, &m_scene->getRegistry());
    m_testCube.AddComponent<MeshRendererComponent>();
    m_cubeMesh = CubeMesh();
    m_testCube.GetComponent<MeshRendererComponent>().mesh = &m_cubeMesh;
    
    entt::entity empty = m_scene->createEntity("Empty Entity");
    m_scene->setParent(empty, cube_id);

    m_selectedEntity = m_editorCamera.GetEntity();
    m_bootstrapped = true;
}

void EditorLayer::OnUpdate(float deltaTime) {
    m_editorCamera.OnUpdate(deltaTime, m_viewport.IsActive());
}

void EditorLayer::OnEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<KeyPressEvent>(KB_BIND_EVENT_FN(EditorLayer::OnKeyPress));
    dispatcher.Dispatch<KeyPressEvent>(KB_BIND_EVENT_FN(EditorLayer::GizmoControls));
}

bool EditorLayer::OnKeyPress(KeyPressEvent& e) {
    return m_editorCamera.OnKeyPress(e);
}

bool EditorLayer::GizmoControls(KeyPressEvent& e) {
    if(!m_editorCamera.IsCapturingMouse()) {
        if (e.getKeyCode() == Key::W) {
            m_gizmoSettings.gizmoType = GizmoSettings::GizmoType::Translate;
            return true;
        }
        if (e.getKeyCode() == Key::E) {
            m_gizmoSettings.gizmoType = GizmoSettings::GizmoType::Rotate;
            return true;
        }
        if (e.getKeyCode() == Key::R) {
            m_gizmoSettings.gizmoType = GizmoSettings::GizmoType::Scale;
            return true;
        }
        if (e.getKeyCode() == Key::Q) {
            m_gizmoSettings.gizmoType = GizmoSettings::GizmoType::None;
            return true;
        }
    }
    return false;
}

void EditorLayer::onImGuiRender() {
    if (!m_styleInitialized) {
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 2.0f;
        style.FrameRounding = 2.0f;
        style.GrabRounding = 2.0f;
        style.TabRounding = 2.0f;
        style.WindowBorderSize = 0.0f;
        style.FrameBorderSize = 0.0f;
        style.WindowPadding = ImVec2(4, 4);
        style.FramePadding = ImVec2(6, 4);
        m_styleInitialized = true;
    }

    ImGuizmo::BeginFrame();
    setupDockSpace();
    drawMenuBar();

    m_hierarchyPanel.Draw(m_scene, &m_selectedEntity);
    m_inspectorPanel.Draw(m_scene, &m_selectedEntity);
    m_contentBrowserPanel.Draw(m_scene);
    m_statsPanel.Draw(m_scene);

    m_viewport.Draw(m_scene, [this]() {
        drawGizmos(m_scene);
    });
}

void EditorLayer::drawGizmos(Scene* scene) {
    if (!scene) return;

    // Target the Viewport window's drawlist
    ImGuizmo::SetDrawlist();

    // Compute the exact screen-space content rectangle
    ImVec2 windowPos  = ImGui::GetWindowPos();
    ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
    ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
    ImGuizmo::SetRect(
        windowPos.x + contentMin.x,
        windowPos.y + contentMin.y,
        contentMax.x - contentMin.x,
        contentMax.y - contentMin.y
    );

    CameraComponent* cameraComponent = &scene->getRegistry().get<CameraComponent>(m_editorCamera.GetEntity());
    Camera& camera = cameraComponent->camera;

    // Grid
    glm::mat4 identity = glm::mat4(1.0f);
    ImGuizmo::DrawGrid(
        glm::value_ptr(camera.getViewMatrix()),
        glm::value_ptr(camera.getProjectionMatrix()),
        glm::value_ptr(identity),
        100.0f
    );

    if (m_selectedEntity == entt::null || m_gizmoSettings.gizmoType == GizmoSettings::GizmoType::None) {
        return;
    }

    auto& transform = scene->getRegistry().get<TransformComponent>(m_selectedEntity);
    glm::mat4 transformMatrix = transform.getLocalMatrix();

    bool manipulated = false;
    switch (m_gizmoSettings.gizmoType) {
        case GizmoSettings::GizmoType::Translate:
            manipulated = ImGuizmo::Manipulate(
                glm::value_ptr(camera.getViewMatrix()),
                glm::value_ptr(camera.getProjectionMatrix()),
                ImGuizmo::OPERATION::TRANSLATE,
                m_gizmoSettings.mode == GizmoSettings::Mode::Local ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD,
                glm::value_ptr(transformMatrix)
            );
            break;
        case GizmoSettings::GizmoType::Rotate:
            manipulated = ImGuizmo::Manipulate(
                glm::value_ptr(camera.getViewMatrix()),
                glm::value_ptr(camera.getProjectionMatrix()),
                ImGuizmo::OPERATION::ROTATE,
                m_gizmoSettings.mode == GizmoSettings::Mode::Local ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD,
                glm::value_ptr(transformMatrix)
            );
            break;
        case GizmoSettings::GizmoType::Scale:
            manipulated = ImGuizmo::Manipulate(
                glm::value_ptr(camera.getViewMatrix()),
                glm::value_ptr(camera.getProjectionMatrix()),
                ImGuizmo::OPERATION::SCALE,
                m_gizmoSettings.mode == GizmoSettings::Mode::Local ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD,
                glm::value_ptr(transformMatrix)
            );
            break;
    }

    if (manipulated) {
        transform.position = glm::vec3(transformMatrix[3]);

        glm::vec3 scale;
        scale.x = glm::length(glm::vec3(transformMatrix[0]));
        scale.y = glm::length(glm::vec3(transformMatrix[1]));
        scale.z = glm::length(glm::vec3(transformMatrix[2]));

        if (scale.x > 0.0f && scale.y > 0.0f && scale.z > 0.0f) {
            glm::mat3 rotationMat(
                glm::vec3(transformMatrix[0]) / scale.x,
                glm::vec3(transformMatrix[1]) / scale.y,
                glm::vec3(transformMatrix[2]) / scale.z
            );
            transform.rotation = glm::quat_cast(rotationMat);
            transform.scale = scale;
        }
    }
}

void EditorLayer::setupDockSpace() {
    static bool dockspaceOpen = true;
    static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Dockerspace", &dockspaceOpen, windowFlags);
    ImGui::PopStyleVar(3);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspaceID = ImGui::GetID("Dockerspace");
        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);
    }

    ImGui::End();
}

void EditorLayer::drawMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {}
            if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {}
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Build", "Ctrl+B")) {}
            if (ImGui::MenuItem("Exit")) {}
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Settings")) {}
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Hierarchy", nullptr, m_hierarchyPanel.OpenFlag());
            ImGui::MenuItem("Inspector", nullptr, m_inspectorPanel.OpenFlag());
            ImGui::MenuItem("Content Browser", nullptr, m_contentBrowserPanel.OpenFlag());
            ImGui::MenuItem("Stats", nullptr, m_statsPanel.OpenFlag());
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("GameObject")) {
            if (ImGui::MenuItem("Create Empty")) {}
            if (ImGui::BeginMenu("3D Object")) {
                if (ImGui::MenuItem("Cube")) {}
                if (ImGui::MenuItem("Sphere")) {}
                if (ImGui::MenuItem("Plane")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Light")) {
                if (ImGui::MenuItem("Point Light")) {}
                if (ImGui::MenuItem("Directional Light")) {}
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}
}