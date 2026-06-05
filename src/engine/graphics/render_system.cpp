#include "render_system.h"
#include "../scene/components/meshrenderer_component.h"
#include "../scene/components/transform.h"
#include "texture.h"
#include "../core/application.h"

namespace Karbon {

RenderSystem::RenderSystem(entt::registry *registry, MaterialSystem *materials, LightingSystem* lights) : m_registry(registry), m_materials(materials), m_lights(lights) {
    if(!m_skyboxShader) {
        m_skyboxShader = std::unique_ptr<Shader>(new Shader("resources/shaders/skybox.vert", "resources/shaders/skybox.frag"));
    }
}
RenderSystem::~RenderSystem() { }
void RenderSystem::Draw(Shader* shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos) { 
    m_materials->uploadToGPU();
    m_lights->Update();

    //Draw skybox first
    auto& activeScene = Application::Get().getActiveScene();
    activeScene.renderSkybox(*m_skyboxShader);

    //Draw rest of scene
    shader->bind();
    shader->bindUniform(view, "view");
    shader->bindUniform(projection, "projection");
    shader->bindUniform(viewPos, "viewPos");
    
    GLuint lightsBlock = glGetUniformBlockIndex(shader->getID(), "Lights");
    if(lightsBlock != GL_INVALID_INDEX) {
        glUniformBlockBinding(shader->getID(), lightsBlock, 1);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_lights->getUBO());
    }

    GLuint materialsBlock = glGetUniformBlockIndex(shader->getID(), "Materials");
    if(materialsBlock != GL_INVALID_INDEX) {
        glUniformBlockBinding(shader->getID(), materialsBlock, 3);
        glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_materials->getUBO());
    }

    auto entt_view = m_registry->view<MeshRendererComponent, WorldTransformComponent>();
    for (auto entity : entt_view) {
        const auto& transform = entt_view.get<WorldTransformComponent>(entity);
        const auto& renderer = entt_view.get<MeshRendererComponent>(entity);

        if (!renderer.visible || !renderer.mesh) continue;
        
        shader->bindUniform(transform.matrix, "model");
        shader->bindUniform(static_cast<int>(renderer.material), "materialIndex");
        m_materials->bindMaterialTextures(renderer.material);
        renderer.mesh->draw(); 
    }
    shader->unbind();
    
}

}

