#pragma once

#include "event.h"
#include "keycode.h"

namespace Karbon {

class KeyEvent : public Event {
public:
    KeyCode() getKeyCode() const { return m_key; }
    EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
protected:
    KeyEvent(const KeyCode key) : m_key(key) {}
    KeyCode m_key;
};

public class KeyPressEvent : public KeyEvent {
    KeyPressedEvent(const KeyCode key) : KeyEvent(keycode) {}
    EVENT_CLASS_TYPE(KeyPress);
};

public class KeyReleaseEvent : public KeyEvent {
    KeyPressedEvent(const KeyCode key) : KeyEvent(keycode) {}
    EVENT_CLASS_TYPE(KeyPress);
};

}