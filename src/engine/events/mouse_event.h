#pragma once
#include "mousecode.h"
#include "event.h"

namespace Karbon {

class MouseMoveEvent : public Event {
public:
    MouseMoveEvent(const float x, const float y) : m_mouseX(x), m_mouseY(y) {}
    EVENT_CLASS_TYPE(MouseMove);
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput);
private:
    float m_mouseX, m_mouseY;
};


class MouseScrollEvent : public Event {
public:
    MouseScrollEvent(const float xOffset, const float yOffset) : m_xOffset(x_Offset), m_yOffset(y_Offset) {}
    EVENT_CLASS_TYPE(MouseScroll);
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput);
private:
    float m_xOffset, m_yOffset;
};


class MouseButtonEvent : public Event {
public:
    MouseCode getMouseButton() const { return m_button; }
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)
protected:
    MouseButtonEvent(const MouseCode button) : m_button(button) {}
    MouseCode m_button;
};



class MouseButtonPressEvent : public Event {
public:
    MouseButtonPressEvent(const MouseCode button) : MouseButtonEvent(button) {}
    EVENT_CLASS_TYPE(MouseButtonPress);
};


class MouseButtonReleaseEvent : public Event {
public:
    MouseButtonReleaseEvent(const MouseCode button) : MouseButtonEvent(button) {}
    EVENT_CLASS_TYPE(MouseButtonRelease);
};

};