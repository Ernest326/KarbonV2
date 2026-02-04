#include <iostream>
#include "window.h"

namespace Karbon {

class Application {
public:
    Application(const char* title);
    virtual ~Application();
    virtual void run();
    void quit();

    void OnEvent(Event& e);

    static Application& Get();
    inline Window& GetWindow() { return *m_window; }

private:
    bool OnWindowClose(WindowCloseEvent& e);
    bool OnWindowResize(WindowResizeEvent& e);
    bool OnKeyPress(KeyPressEvent& e);)

private:
    Window* m_window;
    bool m_running = true;
    bool m_minimised = false;
    static Application* s_Instance;

};

}