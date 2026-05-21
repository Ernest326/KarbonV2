//Input system for any class to use mouse/keyboard/gamepad input, also allow for input axis implementation
//Use the event dispatcher in Application to feed this shared state store.

#pragma once

#include <array>
#include <utility>
#include "../events/keycode.h"
#include "../events/mousecode.h"
#include "../events/key_event.h"
#include "../events/mouse_event.h"
#include "../events/event.h"

namespace Karbon {
    class InputSystem {
    public:
        static InputSystem& Get();

        InputSystem() = default;
        ~InputSystem() = default;

        void BeginFrame();
        void OnEvent(Event& e);

        //Keyboard input
        bool isKeyPressed(int keycode);
        bool isKeyReleased(int keycode);

        //Mouse input
        bool isMouseButtonPressed(int button);
        bool isMouseButtonReleased(int button);
        std::pair<double, double> getMousePosition();
        std::pair<double, double> getMouseDelta();
        std::pair<float, float> getScrollDelta();

        //Gamepad input (if supported)
        bool isGamepadButtonPressed(int button);
        bool isGamepadButtonReleased(int button);
        std::pair<float, float> getGamepadAxis(int axis);

    private:
        bool handleKeyPress(KeyPressEvent& e);
        bool handleKeyRelease(KeyReleaseEvent& e);
        bool handleMouseButtonPress(MouseButtonPressEvent& e);
        bool handleMouseButtonRelease(MouseButtonReleaseEvent& e);
        bool handleMouseMove(MouseMoveEvent& e);
        bool handleMouseScroll(MouseScrollEvent& e);

    private:
        std::array<bool, 512> m_keyStates{};
        std::array<bool, 16> m_mouseButtonStates{};
        double m_mouseX = 0.0;
        double m_mouseY = 0.0;
        double m_mouseDeltaX = 0.0;
        double m_mouseDeltaY = 0.0;
        float m_scrollX = 0.0f;
        float m_scrollY = 0.0f;
    };
}
