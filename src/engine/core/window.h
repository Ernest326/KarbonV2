#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../events/event.h"

namespace Karbon {

struct WindowProperties {
    const char* title;
    unsigned int width, height;
    WindowProperties(const char* title="Karbon", unsigned int width=800, unsigned int height=600) : title(title), width(width), height(height) {};
};

class Window {
public:
    using EventCallbackFn = std::function<void(Event&)>; //Event callback function macro

    Window(const WindowProperties& properties);
    ~Window();
    inline GLFWwindow* getGLWindow() { return m_window };

    inline unsigned int getWidth() const { return m_data.width; }
    inline unsigned int getHeight() const { return m_data.height; }
    inline bool getVSync() const { return m_data.VSync; }
    inline bool close() const { return glfwWindowShouldClose(m_window); }

    inline void setTitle(const char* title) { glfwSetWindowTitle(m_window, title); }
    void setVSync(bool enabled);

    void clear();
    void update();

private:
    virtual bool init(const WindowProperties& properties);
    virtual void shutdown();

private:
    GLFWwindow* m_window;
    struct WindowData {
        const char* title;
        unsigned int width, height;
        bool VSync;
    };
    WindowData m_data;
};

}