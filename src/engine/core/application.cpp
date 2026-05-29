#include "application.h"
#include "../events/application_event.h"
#include "../input/inputsystem.h"
#include "base.h"
#include <iostream>
#include "../graphics/shader.h"
#include "../graphics/texture.h"

#include "../ui/kbimgui.h"

#include "../graphics/spectator_camera.h"

// Physics incldues
#include "../physics/physics_system.h"
#include "../scene/components/collider_component.h"
#include "../scene/components/rigidbody_component.h"
#include "../scene/components/transform.h"
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/RegisterTypes.h>

//Lighting
#include "../graphics/lighting_system.h"

//Rendering
#include "../graphics/render_system.h"
#include "../graphics/material_system.h"

#include "testlayer.h"

namespace Karbon {

float deltaTime = 0.0f;
Application *Application::s_instance = nullptr;
Application &Application::Get() { return *Application::s_instance; }

Application::Application(const char *title) {
    //Window setup
    s_instance = this;
    WindowProperties specification(title);
    m_window = std::make_unique<Window>(specification);
    m_window->setEventCallback(KB_BIND_EVENT_FN(Application::OnEvent));

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
    m_physicsSystem->Initialize();
    m_lightingSystem = std::make_unique<LightingSystem>(&m_activeScene->getRegistry());
    m_lightingSystem->Initialize();
    m_materialSystem = std::make_unique<MaterialSystem>();
    m_renderSystem = std::make_unique<RenderSystem>(&m_activeScene->getRegistry(), m_materialSystem.get(), m_lightingSystem.get());
}

Application::~Application() {}

void Application::run() {

    std::cout << "Running application..." << std::endl;

    KarbonImGUI::init();

    SpectatorCamera camera(glm::vec3(0.0f, 0.0f, 5.0f),
                             glm::vec3(0.0f, 0.0f, 0.0f));
    m_activeScene->setPrimaryCamera(&camera.getCamera());
    Shader test_shader("resources/shaders/test_standard.vert",
                         "resources/shaders/test_standard.frag");

    bool enableTestLayer = true;
    if (enableTestLayer) {
        m_layerStack->pushLayer(new TestLayer(m_activeScene.get(), m_renderSystem.get(), m_physicsSystem.get(),
                                    m_lightingSystem.get(), m_materialSystem.get(), &camera));
    }

    // Some defaults
    float lastFrameTime = getTime();

    // GL defaults
    glEnable(GL_DEPTH_TEST);

    // Game loop
    while (m_running) {
        float currentFrameTime = getTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        InputSystem::Get().BeginFrame();
        glfwPollEvents();

        camera.update(deltaTime);
        m_window->clear();
        KarbonImGUI::begin();

        if (!m_minimised) {

            m_layerStack->update(deltaTime);

            GLuint lights = glGetUniformBlockIndex(test_shader.getID(), "Lights");
            glUniformBlockBinding(test_shader.getID(), lights, 1);
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_lightingSystem->getUBO());

            m_layerStack->imGuiRender();

            //Updates
            m_activeScene->onUpdate();
            m_physicsSystem->Update(deltaTime);
            m_lightingSystem->Update();
            m_renderSystem->Draw(&test_shader, m_activeScene->getPrimaryCamera().getViewMatrix(), m_activeScene->getPrimaryCamera().getProjectionMatrix(), m_activeScene->getPrimaryCamera().getPosition());

        }

        KarbonImGUI::end();
        m_window->update();
    }

    // Shutdown
    m_physicsSystem->Shutdown();
    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;

    KarbonImGUI::shutdown();
    std::cout << "Closing application..." << std::endl;
}

void Application::quit() { m_running = false; }

bool Application::OnWindowClose(WindowCloseEvent &e) {
    quit();
    return true;
}

bool Application::OnWindowResize(WindowResizeEvent &e) {
    if (e.getWidth() == 0 || e.getHeight() == 0) {
        m_minimised = true;
        return false;
    }
    m_minimised = false;
    return false;
}

bool Application::OnKeyPress(KeyPressEvent &e) {
    if (e.getKeyCode() == GLFW_KEY_Q)
        quit();
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

void Application::OnEvent(Event &e) {
    InputSystem::Get().OnEvent(e);
    EventDispatcher dispatcher(e);
    m_layerStack->event(e);
    dispatcher.Dispatch<WindowCloseEvent>(
        KB_BIND_EVENT_FN(Application::OnWindowClose));
    dispatcher.Dispatch<WindowResizeEvent>(
        KB_BIND_EVENT_FN(Application::OnWindowResize));
    dispatcher.Dispatch<KeyPressEvent>(KB_BIND_EVENT_FN(Application::OnKeyPress));
}

float Application::getTime() { return static_cast<float>(glfwGetTime()); }

float Application::getDeltaTime() { return deltaTime; }

} // namespace Karbon