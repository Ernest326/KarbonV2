#include <iostream>
#include <memory>
#include "window.h"
#include "layer.h"

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
    bool OnKeyPress(KeyPressEvent& e);

private:
    // template<typename TLayer>
    // requires(std::is_base_of<Layer, TLayer>)
    // TLayer* GetLayer()
    // {
    //     for (const auto& layer : m_layerStack) {
    //         if (auto casted = dynamic_cast<TLayer*>(layer.get()))
    //             return casted;
    //     }
    //     return nullptr;
    // }

private:
    std::unique_ptr<Window> m_window;
    bool m_running = true;
    bool m_minimised = false;
    static Application* s_instance;

    std::vector<std::unique_ptr<Layer>> m_layerStack;

};

}