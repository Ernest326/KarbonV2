#include "application.h"
#include "base.h"
#include "../events/application_event.h"
#include "../graphics/shader.h"
#include "../graphics/buffers/buffers.h"

namespace Karbon {

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
       
        GLfloat* vertices = new GLfloat[9] {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };

        GLfloat* colors = new GLfloat[9] {
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f
        };

        VBO vertexBuffer(vertices, 9*sizeof(GLfloat));
        VBO colorBuffer(colors, 9*sizeof(GLfloat));
        VAO vertexArray;
        vertexArray.addBuffer(vertexBuffer, 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
        vertexArray.addBuffer(colorBuffer, 1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);

        Shader test_shader("resources/test_triangle.vert", "resources/test_triangle.frag");

        while(m_running) {
            glfwPollEvents();
            m_window->clear();
            if(!m_minimised) {
                //Loop

                //Create a triangle
                test_shader.bind();
                vertexArray.bind();
                glDrawArrays(GL_TRIANGLES, 0, 3);
                test_shader.unbind();
                vertexArray.unbind();

            }
            m_window->update();
            
        }
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
        return true;
    }

    void Application::OnEvent(Event& e) {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(KB_BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(KB_BIND_EVENT_FN(Application::OnWindowResize));
        dispatcher.Dispatch<KeyPressEvent>(KB_BIND_EVENT_FN(Application::OnKeyPress));
    }

}