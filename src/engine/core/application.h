#pragma once

#include <entt/entt.hpp>
#include "../scene/scene.h"
#include "../physics/physics_system.h"
#include "../graphics/render_system.h"
#include "../graphics/lighting_system.h"
#include "../graphics/material_system.h"
#include <memory>
#include "window.h"
#include "layer_stack.h"

#ifndef STBI_IMAGE_IMPLEMENTATION
#define STBI_IMAGE_IMPLEMENTATION
#endif

namespace JPH {
    class JobSystemThreadPool;
}

namespace Karbon {

class Application {
public:
    Application(const char* title);
    virtual ~Application();
    virtual void run();
    void quit();

    void OnEvent(Event& e);

    static Application& Get();
    inline Window& getWindow() { return *m_window; }

    void pushLayer(Layer* layer);
    void pushOverlay(Layer* overlay);

    Scene& getActiveScene();
    RenderSystem& getRenderSystem();
    PhysicsSystem& getPhysicsSystem();
    LightingSystem& getLightingSystem();
    MaterialSystem& getMaterialSystem();

    static float getTime();
    static float getDeltaTime();

private:
    bool OnWindowClose(WindowCloseEvent& e);
    bool OnWindowResize(WindowResizeEvent& e);
    bool OnKeyPress(KeyPressEvent& e);

private:
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Scene> m_activeScene;

    std::unique_ptr<RenderSystem> m_renderSystem;
    std::unique_ptr<PhysicsSystem> m_physicsSystem;
    std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;
    std::unique_ptr<LightingSystem> m_lightingSystem;
    std::unique_ptr<MaterialSystem> m_materialSystem;

    bool m_running = true;
    bool m_minimised = false;
    static Application* s_instance;

    std::unique_ptr<LayerStack> m_layerStack;

};

}