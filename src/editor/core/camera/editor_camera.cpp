#include "core/camera/editor_camera.h"

#include "core/application.h"
#include "scene/components/camera_component.h"
#include "scene/scene.h"
#include <glm/glm.hpp>

namespace Karbon {

void EditorCamera::Initialize(Scene* scene) {
    m_scene = scene;
    if (!m_scene) {
        return;
    }

    m_entity = m_scene->createEntity("Editor Camera");
    auto& camComp = m_scene->getRegistry().emplace<CameraComponent>(m_entity);
    camComp.camera.setPosition(glm::vec3(0, 0, 5));
    m_scene->setPrimaryCamera(m_entity);

    m_controller = std::make_unique<EditorCameraController>(&camComp.camera, &Application::Get().getWindow());
}

void EditorCamera::OnUpdate(float deltaTime, bool viewportActive) {
    if (!m_controller) {
        return;
    }

    m_controller->SetViewportActive(viewportActive);
    m_controller->OnUpdate(deltaTime);
}

bool EditorCamera::OnKeyPress(KeyPressEvent& e) {
    if (e.getKeyCode() != Key::Escape) {
        return false;
    }

    if (m_controller && m_controller->IsCapturingMouse()) {
        m_controller->Release();
        return true;
    }

    return false;
}

} // namespace Karbon
