#include "core/camera/editor_camera.h"

#include "core/application.h"
#include "scene/components/camera_component.h"
#include "scene/scene.h"
#include <glm/glm.hpp>

namespace Karbon {

void EditorCamera::initialize(Scene* scene) {
    m_scene = scene;
    if (!m_scene) {
        return;
    }

    m_entity = m_scene->createEntity("Editor Camera");
    auto& camComp = m_scene->getRegistry().emplace<CameraComponent>(m_entity);
    camComp.camera.setPosition(glm::vec3(0, 0, 5));
    m_scene->setPrimaryCamera(m_entity);

    m_controller = std::make_unique<EditorCameraController>(&camComp.camera, &Application::get().getWindow());
}

void EditorCamera::onUpdate(float deltaTime, bool viewportActive) {
    if (!m_controller) {
        return;
    }

    m_controller->setViewportActive(viewportActive);
    m_controller->onUpdate(deltaTime);
}

bool EditorCamera::onKeyPress(KeyPressEvent& e) {
    if (e.getKeyCode() != Key::Escape) {
        return false;
    }

    if (m_controller && m_controller->isCapturingMouse()) {
        m_controller->release();
        return true;
    }

    return false;
}

} // namespace Karbon
