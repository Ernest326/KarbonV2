#pragma once

#include <memory>
#include <entt/entt.hpp>
#include "core/camera/editor_camera_controller.h"
#include "events/key_event.h"

namespace Karbon {

class Scene;

class EditorCamera {
public:
    void Initialize(Scene* scene);
    void OnUpdate(float deltaTime, bool viewportActive);
    bool OnKeyPress(KeyPressEvent& e);

    entt::entity GetEntity() const { return m_entity; }

private:
    Scene* m_scene = nullptr;
    entt::entity m_entity = entt::null;
    std::unique_ptr<EditorCameraController> m_controller;
};

} // namespace Karbon
