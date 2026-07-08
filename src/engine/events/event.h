//Event dispatcher class created while using Cherno's Hazel as a reference
#pragma once

#include <string>

namespace Karbon {
enum class EventType {
    None=0,
    WindowClose,
    WindowResize,
    WindowMove,
    MouseMove,
    MouseScroll,
    MouseButton,
    MouseButtonPress,
    MouseButtonRelease,
    KeyPress,
    KeyRelease,
    AppTick,
    AppUpdate,
    AppRender
};

// Bit flags: events combine categories with | and isInCategory tests with &
enum EventCategory
{
    None                     = 0,
    EventCategoryApplication = 1 << 0,
    EventCategoryInput       = 1 << 1,
    EventCategoryKeyboard    = 1 << 2,
    EventCategoryMouse       = 1 << 3,
    EventCategoryMouseButton = 1 << 4
};

#define EVENT_CLASS_TYPE(type)                                                  \
	static EventType getStaticType() { return EventType::type; }                \
	virtual EventType getEventType() const override { return getStaticType(); } \
	virtual const char* getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
	virtual int getCategoryFlags() const override { return category; }

class Event {
protected:
    EventType m_type;
public:
    virtual ~Event() = default;
    bool handled = false;

    virtual EventType getEventType() const = 0;
    virtual const char* getName() const = 0;
    virtual int getCategoryFlags() const = 0;
    virtual std::string toString() const { return getName(); }

    bool isInCategory(EventCategory category) {
        return getCategoryFlags() & category;
    }
};

class EventDispatcher {
public:
    EventDispatcher(Event& event) : m_event(event) {}
    
    template <typename T, typename F>
    bool Dispatch(const F& func) {
        if(m_event.getEventType() == T::getStaticType()) {
            m_event.handled |= func(static_cast<T&>(m_event));
            return true;
        }
        return false;
    }
private:
    Event& m_event;
};
}