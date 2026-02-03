#include "application.h"

namespace Karbon {

    Application::Application(const char* title) {
        m_window = new Window(WindowProperties(title));
    }
    Application::~Application() {}


    void Application::run() {
        std::cout << "Running application..." << std::endl;
        while(!m_window->close()) {
            m_window->clear();
            m_window->update();
        }
        std::cout << "Closing application..." << std::endl;
    }

}