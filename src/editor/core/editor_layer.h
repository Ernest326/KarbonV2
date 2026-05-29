#pragma once

#include <entt/entt.hpp>
#include "core/layer.h"
#include "scene/scene.h"

namespace Karbon {

class EditorLayer : public Layer {
public:
    explicit EditorLayer(Scene* scene);

    void onAttach() override;
    void onImGuiRender() override;

private:
    Scene* m_scene = nullptr;
    entt::entity m_selectedEntity = entt::null;
    bool m_bootstrapped = false;
};

}