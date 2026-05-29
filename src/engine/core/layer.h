#pragma once
#include "../events/event.h"

namespace Karbon {
class Layer {

public:
    virtual ~Layer() = default;

    virtual void onAttach() {};
    virtual void onDetach() {};

    virtual void OnUpdate(float deltaTime) {};
    virtual void OnEvent(Event& e) {}
    virtual void onImGuiRender() {}
    virtual void OnRender() {}
};
}