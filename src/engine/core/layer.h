#pragma once
#include "../events/event.h"

namespace Karbon {
class Layer {

public:
    Layer() = default;
    virtual void OnUpdate();
    virtual void OnEvent(Event& e);
    virtual void OnRender();
    
};
}