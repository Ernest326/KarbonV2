#pragma once
#include "layer.h"
#include <vector>

namespace Karbon {

class LayerStack {
public:
    ~LayerStack();

    void pushLayer(Layer* layer);
    void pushOverlay(Layer* overlay);
    void popLayer(Layer* layer);
    void popOverlay(Layer* overlay);

    std::vector<Layer*>::iterator begin() { return m_layers.begin(); }
    std::vector<Layer*>::iterator end() { return m_layers.end(); }
    std::vector<Layer*>::reverse_iterator rbegin() { return m_layers.rbegin(); }
    std::vector<Layer*>::reverse_iterator rend() { return m_layers.rend(); }

    void event(Event& e);
    void update(float deltaTime);
    void imGuiRender();

private:
    std::vector<Layer*> m_layers;
    unsigned int m_layerInsertIndex = 0;

};

}