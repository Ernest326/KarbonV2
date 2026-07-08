#include "inputsystem.h"

#include <GLFW/glfw3.h>

namespace Karbon {

InputSystem& InputSystem::get() {
    static InputSystem instance;
    return instance;
}

void InputSystem::beginFrame() {
    m_mouseDeltaX = 0.0;
    m_mouseDeltaY = 0.0;
    m_scrollX = 0.0f;
    m_scrollY = 0.0f;
}

void InputSystem::onEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.dispatch<KeyPressEvent>([this](KeyPressEvent& event) { return handleKeyPress(event); });
    dispatcher.dispatch<KeyReleaseEvent>([this](KeyReleaseEvent& event) { return handleKeyRelease(event); });
    dispatcher.dispatch<MouseButtonPressEvent>([this](MouseButtonPressEvent& event) { return handleMouseButtonPress(event); });
    dispatcher.dispatch<MouseButtonReleaseEvent>([this](MouseButtonReleaseEvent& event) { return handleMouseButtonRelease(event); });
    dispatcher.dispatch<MouseMoveEvent>([this](MouseMoveEvent& event) { return handleMouseMove(event); });
    dispatcher.dispatch<MouseScrollEvent>([this](MouseScrollEvent& event) { return handleMouseScroll(event); });
}

bool InputSystem::isKeyPressed(int keycode) {
    if (keycode < 0 || keycode >= static_cast<int>(m_keyStates.size())) {
        return false;
    }
    return m_keyStates[static_cast<size_t>(keycode)];
}

bool InputSystem::isKeyReleased(int keycode) {
    return !isKeyPressed(keycode);
}

bool InputSystem::isMouseButtonPressed(int button) {
    if (button < 0 || button >= static_cast<int>(m_mouseButtonStates.size())) {
        return false;
    }
    return m_mouseButtonStates[static_cast<size_t>(button)];
}

bool InputSystem::isMouseButtonReleased(int button) {
    return !isMouseButtonPressed(button);
}

std::pair<double, double> InputSystem::getMousePosition() {
    return {m_mouseX, m_mouseY};
}

std::pair<double, double> InputSystem::getMouseDelta() {
    return {m_mouseDeltaX, m_mouseDeltaY};
}

std::pair<float, float> InputSystem::getScrollDelta() {
    return {m_scrollX, m_scrollY};
}

bool InputSystem::isGamepadButtonPressed(int button) {
    GLFWgamepadstate state{};
    for (int joystick = GLFW_JOYSTICK_1; joystick <= GLFW_JOYSTICK_LAST; ++joystick) {
        if (!glfwJoystickIsGamepad(joystick)) {
            continue;
        }
        if (!glfwGetGamepadState(joystick, &state)) {
            continue;
        }
        if (button >= 0 && button <= GLFW_GAMEPAD_BUTTON_LAST) {
            return state.buttons[button] == GLFW_PRESS;
        }
        return false;
    }
    return false;
}

bool InputSystem::isGamepadButtonReleased(int button) {
    return !isGamepadButtonPressed(button);
}

std::pair<float, float> InputSystem::getGamepadAxis(int axis) {
    GLFWgamepadstate state{};
    for (int joystick = GLFW_JOYSTICK_1; joystick <= GLFW_JOYSTICK_LAST; ++joystick) {
        if (!glfwJoystickIsGamepad(joystick)) {
            continue;
        }
        if (!glfwGetGamepadState(joystick, &state)) {
            continue;
        }

        if (axis < 0 || axis >= GLFW_GAMEPAD_AXIS_LAST) {
            return {0.0f, 0.0f};
        }

        int nextAxis = axis + 1;
        float first = state.axes[axis];
        float second = 0.0f;
        if (nextAxis < GLFW_GAMEPAD_AXIS_LAST) {
            second = state.axes[nextAxis];
        }
        return {first, second};
    }
    return {0.0f, 0.0f};
}

bool InputSystem::handleKeyPress(KeyPressEvent& e) {
    const int key = static_cast<int>(e.getKeyCode());
    if (key >= 0 && key < static_cast<int>(m_keyStates.size())) {
        m_keyStates[static_cast<size_t>(key)] = true;
    }
    return false;
}

bool InputSystem::handleKeyRelease(KeyReleaseEvent& e) {
    const int key = static_cast<int>(e.getKeyCode());
    if (key >= 0 && key < static_cast<int>(m_keyStates.size())) {
        m_keyStates[static_cast<size_t>(key)] = false;
    }
    return false;
}

bool InputSystem::handleMouseButtonPress(MouseButtonPressEvent& e) {
    const int button = static_cast<int>(e.getMouseButton());
    if (button >= 0 && button < static_cast<int>(m_mouseButtonStates.size())) {
        m_mouseButtonStates[static_cast<size_t>(button)] = true;
    }
    return false;
}

bool InputSystem::handleMouseButtonRelease(MouseButtonReleaseEvent& e) {
    const int button = static_cast<int>(e.getMouseButton());
    if (button >= 0 && button < static_cast<int>(m_mouseButtonStates.size())) {
        m_mouseButtonStates[static_cast<size_t>(button)] = false;
    }
    return false;
}

bool InputSystem::handleMouseMove(MouseMoveEvent& e) {
    const double newMouseX = e.getX();
    const double newMouseY = e.getY();
    m_mouseDeltaX += newMouseX - m_mouseX;
    m_mouseDeltaY += newMouseY - m_mouseY;
    m_mouseX = newMouseX;
    m_mouseY = newMouseY;
    return false;
}

bool InputSystem::handleMouseScroll(MouseScrollEvent& e) {
    m_scrollX += e.getXOffset();
    m_scrollY += e.getYOffset();
    return false;
}

}