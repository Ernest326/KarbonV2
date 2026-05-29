#pragma once

#include <entt/entt.hpp>
#include "core/layer.h"
#include "scene/scene.h"

namespace Karbon {

class EditorLayer : public Layer {
public:
    explicit EditorLayer(Scene* scene);

    void onAttach() override;
    void onImGuiRender() override;

private:

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

    Scene* m_scene = nullptr;
    entt::entity m_selectedEntity = entt::null;
    bool m_bootstrapped = false;
    bool m_styleInitialized = false;
};

}