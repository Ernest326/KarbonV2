#include "render_system.h"
#include "../scene/components/meshrenderer_component.h"
#include "../scene/components/transform.h"
#include "texture.h"

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
    if(m_skybox) {
        glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
        glDepthMask(GL_FALSE);
        m_skyboxShader->bind();
        // Remove translation from the view matrix so the skybox stays centered on the camera
        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
        m_skyboxShader->bindUniform(viewNoTranslation, "view");
        m_skyboxShader->bindUniform(projection, "projection");
        m_skyboxShader->bindUniform(0, "skybox");
        m_skybox->bind(0);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
        m_skybox->unbind();
        m_skyboxShader->unbind();
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS); // Set depth function back to default
    }

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
        m_materials->bindAlbedoMap(renderer.material, 2);
        shader->bindUniform(2, "albedoMap");
        m_materials->bindNormalMap(renderer.material, 3);
        shader->bindUniform(3, "normalMap");
        renderer.mesh->draw();
    }
    shader->unbind();
    
}

}

