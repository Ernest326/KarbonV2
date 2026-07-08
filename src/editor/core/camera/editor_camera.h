#pragma once

#include <memory>
#include <entt/entt.hpp>
#include "core/camera/editor_camera_controller.h"
#include "events/key_event.h"

namespace Karbon {

class Scene;

class EditorCamera {
public:
    void initialize(Scene* scene);
    void onUpdate(float deltaTime, bool viewportActive);
    bool onKeyPress(KeyPressEvent& e);
    bool isCapturingMouse() const { return m_controller ? m_controller->isCapturingMouse() : false; }

    entt::entity getEntity() const { return m_entity; }

private:
    Scene* m_scene = nullptr;
    entt::entity m_entity = entt::null;
    std::unique_ptr<EditorCameraController> m_controller;
};

} // namespace Karbon
