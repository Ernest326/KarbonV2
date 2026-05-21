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

        GLfloat* cube_verts = new GLfloat[108] {
            // Front face
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            // Back face
            -0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            // Left face
            -0.5f, -0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            // Right face
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
            // Top face
            -0.5f,  0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            // Bottom face
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f
        };

        GLuint indices[36] = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4,
            8, 9, 10, 10, 11, 8,
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
            20, 21, 22, 22, 23, 20
        };

        GLfloat* glTexCoords = new GLfloat[72] {
            // Front face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            // Back face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            // Left face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            // Right face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            // Top face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            // Bottom face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
        };

        IBO indexBuffer(indices, 36);
        Shader test_shader("resources/test_texture.vert", "resources/test_texture.frag");
        VBO vertexBuffer(cube_verts, 108*sizeof(GLfloat));
        VAO vertexArray;
        vertexArray.addBuffer(vertexBuffer, 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
        VBO texCoordBuffer(glTexCoords, 72*sizeof(GLfloat));
        vertexArray.addBuffer(texCoordBuffer, 1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);

        Texture test_texture("resources/texture.png");

        glm::mat4 model = glm::mat4(1.0f);

        SpectatorCamera camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f));

        float lastFrameTime = getTime();

        glEnable(GL_DEPTH_TEST);

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
                //Loop
                model = glm::mat4(1.0f);
                model = glm::rotate(model, static_cast<float>(glfwGetTime()), glm::vec3(1.0f, 1.0f, 0.0f));
                test_shader.bind();
                test_shader.bindUniform(model, "model");
                test_shader.bindUniform(camera.getViewMatrix(), "view");
                test_shader.bindUniform(camera.getProjectionMatrix(), "projection");
                test_texture.bind(0);
                test_shader.bind();
                vertexArray.bind();
                indexBuffer.bind();
                glDrawElements(GL_TRIANGLES, indexBuffer.getCount(), GL_UNSIGNED_INT, nullptr);
                indexBuffer.unbind();
                vertexArray.unbind();
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