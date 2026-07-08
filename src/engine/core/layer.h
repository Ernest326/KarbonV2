#pragma once
#include "events/event.h"

namespace Karbon {
class Layer {

public:
    virtual ~Layer() = default;

    virtual void onAttach() {};
    virtual void onDetach() {};

    virtual void onUpdate(float deltaTime) {};
    virtual void onEvent(Event& e) {}
    virtual void onImGuiRender() {}
    virtual void onRender() {}
};
}