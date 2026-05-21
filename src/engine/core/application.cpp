#include "application.h"
#include "base.h"
#include <iostream>
#include "../events/application_event.h"
#include "../input/inputsystem.h"
#include "../graphics/shader.h"
#include "../graphics/buffers/buffers.h"
#include "../graphics/texture.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../ui/kbimgui.h"
#include "../graphics/spectator_camera.h"
#include "../graphics/cube.h"

namespace Karbon {

    float deltaTime = 0.0f;
    Application* Application::s_instance = nullptr;
    Application& Application::Get() { return *Application::s_instance; }

    Application::Application(const char* title) {
        s_instance = this;
        WindowProperties specification(title);
        m_window = std::make_unique<Window>(specification);
        m_window->setEventCallback(KB_BIND_EVENT_FN(Application::OnEvent));
    }

    Application::~Application() {}

    void Application::run() {
        std::cout << "Running application..." << std::endl;

        KarbonImGUI::init();

        Shader test_shader("resources/test_texture.vert", "resources/test_texture.frag");
        Texture test_texture("resources/texture.png");
        Cube test_cube(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));

        SpectatorCamera camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f));

        float lastFrameTime = getTime();

        //GL defaults
        glEnable(GL_DEPTH_TEST);

        //Game loop
        while(m_running) {
            float currentFrameTime = getTime();
            deltaTime = currentFrameTime - lastFrameTime;
            lastFrameTime = currentFrameTime;
 
            InputSystem::Get().BeginFrame();
            glfwPollEvents();
            camera.update(deltaTime);
            m_window->clear();
            KarbonImGUI::begin();

            if(!m_minimised) {
                ImGui::Begin("Test Window");
                ImGui::Text("Hello, world!");
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::Text("Position: (%.2f, %.2f, %.2f)", camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
                ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", camera.getRotation().x, camera.getRotation().y, camera.getRotation().z);
                ImGui::End();

                test_cube.setRotation(test_cube.getRotation() + glm::vec3(0.0f, 20.0f * deltaTime, 0.0f));

                test_shader.bind();
                test_shader.bindUniform(test_cube.getModelMatrix(), "model");
                test_shader.bindUniform(camera.getViewMatrix(), "view");
                test_shader.bindUniform(camera.getProjectionMatrix(), "projection");
                test_texture.bind(0);
                test_shader.bind();
                test_cube.draw();
                test_shader.unbind();
                test_texture.unbind();

            }
            KarbonImGUI::end();
            m_window->update();
 
        }
        KarbonImGUI::shutdown();
        std::cout << "Closing application..." << std::endl;
    }

    void Application::quit() {
        m_running=false;
    }

    bool Application::OnWindowClose(WindowCloseEvent& e) {
        quit();
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e) {
        if(e.getWidth() == 0 || e.getHeight() == 0) {
            m_minimised = true;
            return false;
        }
        m_minimised = false;
        //Renderer on resize

        return false;
    }

    bool Application::OnKeyPress(KeyPressEvent& e) {
        if(e.getKeyCode() == GLFW_KEY_Q) quit();
        if(e.getKeyCode() == GLFW_KEY_ESCAPE) {
            //Toggle mouse visible
            GLFWwindow* window = m_window->getGLWindow();
            int mode = glfwGetInputMode(window, GLFW_CURSOR);
            if (mode == GLFW_CURSOR_NORMAL) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
        return true;
    }

    void Application::OnEvent(Event& e) {
        InputSystem::Get().OnEvent(e);
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(KB_BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(KB_BIND_EVENT_FN(Application::OnWindowResize));
        dispatcher.Dispatch<KeyPressEvent>(KB_BIND_EVENT_FN(Application::OnKeyPress));
    }
    
    float Application::getTime() {
        return static_cast<float>(glfwGetTime());
    }

    float Application::getDeltaTime() {
        return deltaTime;
    }

}