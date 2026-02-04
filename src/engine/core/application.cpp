#include "application.h"
#include "base.h"

namespace Karbon {


    Application* Application::s_instance = nullptr;
    Application* Application::Get() { return *Applcation::s_instance; }

    Application::Application(const char* title) {
        m_window = new Window(WindowProperties(title));
    }

    Application::~Application() {} //Shutdown renderer engine

    void Application::run() {
        //
        std::cout << "Running application..." << std::endl;
        while(!m_window->close()) {
            m_window->clear();
            if(!m_minimised) {
                //
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
        return false;
    }

    void Application::OnEvent(Event& e) {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(KB_BIND_EVENT_FN(Application::OnWindowClose()));
        dispatcher.Dispatch<WindowResizeEvent>(KB_BIND_EVENT_FN(Application::OnWindowResize()));
        dispatcher.Dispatch<KeyPressedEvent>(KB_BIND_EVENT_FN(Application::OnKeyPressed()));
    }

}