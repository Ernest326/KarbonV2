#include "application.h"
#include "events/application_event.h"
#include "input/inputsystem.h"
#include "base.h"
#include <iostream>

#include "ui/kbimgui.h"

// Physics includes
#include "physics/physics_system.h"
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/RegisterTypes.h>

// Systems
#include "graphics/lighting_system.h"
#include "graphics/render_system.h"
#include "graphics/material_system.h"

namespace Karbon {

float deltaTime = 0.0f;
Application *Application::s_instance = nullptr;
Application &Application::get() { return *Application::s_instance; }

Application::Application(const char *title) {
    //Window setup
    s_instance = this;
    WindowProperties specification(title);
    m_window = std::make_unique<Window>(specification);
    m_window->setEventCallback(KB_BIND_EVENT_FN(Application::onEvent));

    // Jolt physics one-time global init
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    //Create Layer Stack
    m_layerStack = std::make_unique<LayerStack>();
    m_jobSystem = std::make_unique<JPH::JobSystemThreadPool>(2048, 8, std::thread::hardware_concurrency() - 1);
    m_activeScene = std::make_unique<Scene>();

    //Startup systems
    m_physicsSystem = std::make_unique<PhysicsSystem>(&m_activeScene->getRegistry(), m_jobSystem.get());
    m_physicsSystem->initialize();
    m_lightingSystem = std::make_unique<LightingSystem>(&m_activeScene->getRegistry());
    m_materialSystem = std::make_unique<MaterialSystem>();
    m_assetManager = std::make_unique<AssetManager>(m_materialSystem.get());
    m_renderSystem = std::make_unique<RenderSystem>(&m_activeScene->getRegistry(), m_materialSystem.get(), m_lightingSystem.get(), m_assetManager.get());
}

Application::~Application() {}

void Application::pushLayer(Layer* layer) {
    if (m_layerStack) {
        m_layerStack->pushLayer(layer);
    }
}

void Application::pushOverlay(Layer* overlay) {
    if (m_layerStack) {
        m_layerStack->pushOverlay(overlay);
    }
}

Scene& Application::getActiveScene() { return *m_activeScene; }
RenderSystem& Application::getRenderSystem() { return *m_renderSystem; }
PhysicsSystem& Application::getPhysicsSystem() { return *m_physicsSystem; }
LightingSystem& Application::getLightingSystem() { return *m_lightingSystem; }
MaterialSystem& Application::getMaterialSystem() { return *m_materialSystem; }
AssetManager& Application::getAssetManager() { return *m_assetManager; }

void Application::run() {

    std::cout << "Running application..." << std::endl;

    KarbonImGUI::init();

    float lastFrameTime = getTime();

    // GL defaults
    glEnable(GL_DEPTH_TEST);

    // Game loop
    while (m_running) {
        float currentFrameTime = getTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        InputSystem::get().beginFrame();
        glfwPollEvents();

        if (!m_viewportFramebuffer) {
            m_window->clear();
        }

        KarbonImGUI::begin();

        if (!m_minimised) {

            //Updates
            m_layerStack->update(deltaTime);  // onAttach() creates entities here
            m_activeScene->onUpdate();         // propagate world transforms (includes newly created entities)

            m_physicsSystem->update(deltaTime);

            m_layerStack->imGuiRender();
            Camera* activeCamera = m_activeScene->getPrimaryCamera();
            if (activeCamera) {
                if(m_viewportFramebuffer) {
                    m_viewportFramebuffer->bind();
                    glClearColor(0.16f, 0.16f, 0.18f, 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                }

                m_renderSystem->draw(*m_activeScene, *activeCamera);
                m_layerStack->render();
                if(m_viewportFramebuffer) {
                    m_viewportFramebuffer->unbind();
                }
            }

        }

        KarbonImGUI::end();
        m_window->update();
    }

    // shutdown
    m_physicsSystem->shutdown();
    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;

    KarbonImGUI::shutdown();
    std::cout << "Closing application..." << std::endl;
}

void Application::quit() { m_running = false; }

bool Application::onWindowClose(WindowCloseEvent &e) {
    quit();
    return true;
}

bool Application::onWindowResize(WindowResizeEvent &e) {
    if (e.getWidth() == 0 || e.getHeight() == 0) {
        m_minimised = true;
        return false;
    }
    m_minimised = false;
    return false;
}

bool Application::onKeyPress(KeyPressEvent &e) {
    /*
    if (e.getKeyCode() == GLFW_KEY_Q)
        quit();
    */
    if (e.getKeyCode() == GLFW_KEY_ESCAPE) {
        // Toggle mouse visible
        GLFWwindow *window = m_window->getGLWindow();
        int mode = glfwGetInputMode(window, GLFW_CURSOR);
        if (mode == GLFW_CURSOR_NORMAL) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    return true;
}

void Application::onEvent(Event &e) {
    InputSystem::get().onEvent(e);
    EventDispatcher dispatcher(e);
    m_layerStack->event(e);
    dispatcher.dispatch<WindowCloseEvent>(
        KB_BIND_EVENT_FN(Application::onWindowClose));
    dispatcher.dispatch<WindowResizeEvent>(
        KB_BIND_EVENT_FN(Application::onWindowResize));
    dispatcher.dispatch<KeyPressEvent>(KB_BIND_EVENT_FN(Application::onKeyPress));
}

float Application::getTime() { return static_cast<float>(glfwGetTime()); }

float Application::getDeltaTime() { return deltaTime; }

} // namespace Karbon