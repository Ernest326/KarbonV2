#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Karbon {

struct WindowProperties {
    const char* title;
    unsigned int width, height;
    WindowProperties(const char* title="Karbon", unsigned int width=800, unsigned int height=600) : title(title), width(width), height(height) {};
};

class Window {
public:
    Window(const WindowProperties& properties);
    ~Window();
    GLFWwindow* getGLWindow();

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