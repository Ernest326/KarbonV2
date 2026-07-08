#include "render_system.h"
#include "../scene/components/meshrenderer_component.h"
#include "../scene/components/transform.h"
#include "../scene/scene.h"
#include "texture.h"

namespace Karbon {

// Uniform block binding points shared with the shaders
constexpr GLuint LIGHTS_BLOCK_BINDING = 1;
constexpr GLuint MATERIALS_BLOCK_BINDING = 3;

RenderSystem::RenderSystem(entt::registry *registry, MaterialSystem *materials, LightingSystem* lights) : m_registry(registry), m_materials(materials), m_lights(lights) {
    m_pbrShader = std::make_unique<Shader>("resources/shaders/pbr.vert", "resources/shaders/pbr.frag");
    m_skyboxShader = std::make_unique<Shader>("resources/shaders/skybox.vert", "resources/shaders/skybox.frag");
}
RenderSystem::~RenderSystem() { }

void RenderSystem::Draw(Scene& scene, const Camera& camera) {
    // GPU upload phase: materials and lights are gathered/uploaded once per frame here
    m_materials->uploadToGPU();
    m_lights->Update();

    //Draw skybox first
    scene.renderSkybox(*m_skyboxShader);

    //Draw rest of scene
    Shader* shader = m_pbrShader.get();
    shader->bind();
    shader->bindUniform(camera.getViewMatrix(), "view");
    shader->bindUniform(camera.getProjectionMatrix(), "projection");
    shader->bindUniform(camera.getPosition(), "viewPos");

    // Bind IBL maps + assign their sampler units via the scene environment.
    scene.getEnvironment().bindIBL(*shader);
    glActiveTexture(GL_TEXTURE0); // leave unit 0 active for material binds

    GLuint lightsBlock = glGetUniformBlockIndex(shader->getID(), "Lights");
    if(lightsBlock != GL_INVALID_INDEX) {
        glUniformBlockBinding(shader->getID(), lightsBlock, LIGHTS_BLOCK_BINDING);
        glBindBufferBase(GL_UNIFORM_BUFFER, LIGHTS_BLOCK_BINDING, m_lights->getUBO());
    }

    GLuint materialsBlock = glGetUniformBlockIndex(shader->getID(), "Materials");
    if(materialsBlock != GL_INVALID_INDEX) {
        glUniformBlockBinding(shader->getID(), materialsBlock, MATERIALS_BLOCK_BINDING);
        glBindBufferBase(GL_UNIFORM_BUFFER, MATERIALS_BLOCK_BINDING, m_materials->getUBO());
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
