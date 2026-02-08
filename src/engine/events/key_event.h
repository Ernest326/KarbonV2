#pragma once
#include "event.h"
#include "keycode.h"

namespace Karbon {

class KeyEvent : public Event {
public:
    KeyCode getKeyCode() const { return m_key; }
    EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
protected:
    KeyEvent(const KeyCode key) : m_key(key) {}
    KeyCode m_key;
};

class KeyPressEvent : public KeyEvent {
public:
    KeyPressEvent(const KeyCode key, bool isRepeat=false) : KeyEvent(key), m_isRepeat(isRepeat) {}
    bool isRepeat() { return m_isRepeat; }
    EVENT_CLASS_TYPE(KeyPress);
private:
    bool m_isRepeat;
};

class KeyReleaseEvent : public KeyEvent {
public:
    KeyReleaseEvent(const KeyCode key) : KeyEvent(key) {}
    EVENT_CLASS_TYPE(KeyPress);
};

}