#include "editor_layer.h"

#include "karbon.h"
#include <iostream>
#include <imgui.h>
#include "scene/cube_mesh.h"
#include "scene/components/meshrenderer_component.h"
#include "scene/components/id_component.h"
#include "scene/components/camera_component.h"
#include "scene/components/pointlight_component.h"

namespace Karbon {

EditorLayer::EditorLayer(Scene* scene) : m_scene(scene) {
}

void EditorLayer::onAttach() {
    //Some basic entity setup for testing
    if (!m_scene || m_bootstrapped) {
        return;
    }

    m_viewportFramebuffer = std::make_unique<Framebuffer>(1280, 720);
    Application::Get().setViewportFramebuffer(m_viewportFramebuffer.get());

    entt::entity camera = m_scene->createEntity("Editor Camera");
    m_scene->getRegistry().emplace<CameraComponent>(camera);
    m_scene->setPrimaryCamera(camera);

    entt::entity directionalLight = m_scene->createEntity("Point Light");
    m_scene->getRegistry().get<TransformComponent>(directionalLight).position = glm::vec3(0.0f, 4.0f, 0.0f);
    m_scene->getRegistry().emplace<PointLightComponent>(directionalLight);
    
    entt::entity cube_id = m_scene->createEntity("Test Cube");
    m_testCube = Entity(cube_id, &m_scene->getRegistry());
    m_testCube.AddComponent<MeshRendererComponent>();
    m_cubeMesh = CubeMesh();
    m_testCube.GetComponent<MeshRendererComponent>().mesh = &m_cubeMesh;

    m_selectedEntity = camera;

    m_bootstrapped = true;
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

    setupDockSpace();
    drawMenuBar();

    if (m_showHierarchy) { drawHierarchy(); }
    if (m_showInspector) { drawInspector(); }
    if (m_showContentBrowser) { drawContentBrowser(); }
    if (m_showStats) { drawStats(); }

    drawViewport();
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
            ImGui::MenuItem("Hierarchy", nullptr, &m_showHierarchy);
            ImGui::MenuItem("Inspector", nullptr, &m_showInspector);
            ImGui::MenuItem("Content Browser", nullptr, &m_showContentBrowser);
            ImGui::MenuItem("Stats", nullptr, &m_showStats);
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

void EditorLayer::drawInspector() {
    ImGui::Begin("Inspector", &m_showInspector);
    
    if (ImGui::Button("Add Component")) {
        ImGui::OpenPopup("AddComponentPopup");
    }
    
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

void EditorLayer::drawHierarchy() {
    ImGui::Begin("Hierarchy", &m_showHierarchy);
    
    if (m_scene) {
        // TODO: Iterate scene entities, display tree
        ImGui::Text("Scene Entities");
        ImGui::Separator();
        
        for(auto entity : m_scene->getRegistry().view<IDComponent>()) {
            auto& id = m_scene->getRegistry().get<IDComponent>(entity);
            auto& tag = m_scene->getRegistry().get<TagComponent>(entity);
            ImGui::Text("%s (ID: %d)", tag.tag.c_str(), id.id);
        }
    } else {
        ImGui::Text("No scene loaded");
    }
    
    ImGui::End();
}

void EditorLayer::drawContentBrowser() {
    ImGui::Begin("Content Browser", &m_showContentBrowser);
    
    // Breadcrumb
    ImGui::Text("assets > meshes");
    ImGui::Separator();
    
    // Asset grid
    float cellSize = 80.0f;
    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = static_cast<int>(panelWidth / cellSize);
    if (columnCount < 1) columnCount = 1;
    
    ImGui::Columns(columnCount, nullptr, false);
    
    // Placeholder items
    for (int i = 0; i < 12; i++) {
        ImGui::Button("file", ImVec2(cellSize - 10, cellSize - 10));
        ImGui::Text("asset_%d.fbx", i);
        ImGui::NextColumn();
    }
    
    ImGui::Columns(1);
    ImGui::End();
}

void EditorLayer::drawStats() {
    ImGui::Begin("Stats", &m_showStats);
    
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::Text("Draw Calls: 0");
    ImGui::Text("Triangles: 0");
    ImGui::Text("Entities: 0");
    ImGui::Separator();
    //ImGui::Text("Viewport: %.0f x %.0f", m_viewportSize.x, m_viewportSize.y);
    
    ImGui::End();
}

void EditorLayer::drawViewport() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    m_viewportFocused = ImGui::IsWindowFocused();
    m_viewportHovered = ImGui::IsWindowHovered();

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    m_viewportSize = viewportPanelSize;

    uint32_t textureID = m_viewportFramebuffer->getColorAttachment();
    ImGui::Image((void*)(uintptr_t)textureID, viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
    ImGui::PopStyleVar();
}

}