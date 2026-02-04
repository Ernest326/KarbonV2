#include "window.h"

namespace Karbon {

static bool s_GLFWInit = false;
static bool s_GladInit = false;

Window::Window(const WindowProperties& properties) {
    init(properties);
}

Window::~Window() {
    shutdown();
}

bool Window::init(const WindowProperties& properties) {
    m_data.title = properties.title;
    m_data.width = properties.width;
    m_data.height = properties.height;

    //Load GLFW if it hasnt been loaded yet
    if (!s_GLFWInit) {
        if(!glfwInit()) {
            std::cout << "ERROR::Failed to initialize GLFW!!!" << std::endl;
            return false;
        }
        s_GLFWInit = true;
    }

    m_window = glfwCreateWindow(properties.width, properties.height, properties.title, nullptr, nullptr);
    if (!m_window) {
        std::cout << "ERROR::Failed to create Window!!!" << std::endl;
        return false;
    }

    glfwMakeContextCurrent(m_window); //Set new window as current context

    //Same for glad
    if (!s_GladInit) {
        if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "ERROR::Failed to initialize GLAD!!!" << std::endl;
            return false;
        }
        s_GladInit = true;
    }

    glfwSetWindowUserPointer(m_window, &m_data); //Use a window data structure as a pointer
    setVSync(true);

    //Window events
    glfwSetWindowSizeCallback(m_window,
    [](GLFWwindow* window, int width, int height){
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        data.width = width;
        data.height = height;

        WindowResizeEvent event(width, height);
        data.EventCallback(event);
    });
    glfwSetWindowCloseCallback(m_window,
    [](GLFWwindow* window) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        WindowCloseEvent event();
        data.EventCallback(event);
    });
    glfwSetKeyCallback(m_window,
    [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        switch (action) {
            case GLFW_PRESS: {
                KeyPressEvent event(key, 0);
                data.EventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                KeyReleaseEvent event(key);
                data.EventCallback(event);
                break;
            }
        }
    });
    glfwSetMouseButtonCallback(m_window,
    [](GLFWwindow* window) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        switch(action) {
            case GLFW_PRESS: {
                MouseButtonPressEvent event(button);
                data.EventCallback(event);
            }
            case GLFW_RELEASE: {
                MouseButtonPressEvent event(button);
                data.EventCallback(event);
            }
        }
    });
    glfwSetScrollCallback(m_window,
    [](GLFWwindow* window, double xOffset, double yOffset) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        MouseScrollEvent event((float)xOffset, (float)yOffset);
        data.EventCallback(event);
    });
    glfwSetCursorPosCallback(m_window,
    [](GLFWwindow* window, double xPos, double yPos) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        MouseMoveEvent event((float)xPos, (float)yPos);
        data.Eventcallback(event);
    });

    return true;

}

void Window::clear() {
    glClearColor(0.16f, 0.16f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::update() {
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void Window::shutdown() {
    glfwDestroyWindow(m_window);
    glfwTerminate(); //Shutdown the app after window is closed
}

void Window::setVSync(bool enabled) {
    glfwSwapInterval((int)enabled);
    m_data.VSync = enabled;
}

}