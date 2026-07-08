#include "render_system.h"
#include "assets/asset_manager.h"
#include "scene/components/meshrenderer_component.h"
#include "scene/components/transform.h"
#include "scene/scene.h"
#include "texture.h"

namespace Karbon {

// Uniform block binding points shared with the shaders
constexpr GLuint LIGHTS_BLOCK_BINDING = 1;
constexpr GLuint MATERIALS_BLOCK_BINDING = 3;

RenderSystem::RenderSystem(entt::registry *registry, MaterialSystem *materials, LightingSystem* lights, AssetManager* assets) : m_registry(registry), m_materials(materials), m_lights(lights), m_assets(assets) {
    m_pbrShader = std::make_unique<Shader>("resources/shaders/pbr.vert", "resources/shaders/pbr.frag");
    m_skyboxShader = std::make_unique<Shader>("resources/shaders/skybox.vert", "resources/shaders/skybox.frag");

    // Uniform block bindings are program state — assign once at link time
    GLuint lightsBlock = glGetUniformBlockIndex(m_pbrShader->getID(), "Lights");
    if (lightsBlock != GL_INVALID_INDEX) {
        glUniformBlockBinding(m_pbrShader->getID(), lightsBlock, LIGHTS_BLOCK_BINDING);
    }
    GLuint materialsBlock = glGetUniformBlockIndex(m_pbrShader->getID(), "Materials");
    if (materialsBlock != GL_INVALID_INDEX) {
        glUniformBlockBinding(m_pbrShader->getID(), materialsBlock, MATERIALS_BLOCK_BINDING);
    }
}
RenderSystem::~RenderSystem() { }

void RenderSystem::draw(Scene& scene, const Camera& camera) {
    // GPU upload phase: materials and lights are gathered/uploaded once per frame here
    m_materials->uploadToGPU();
    m_lights->update();

    //draw skybox first
    scene.renderSkybox(*m_skyboxShader);

    //draw rest of scene
    Shader* shader = m_pbrShader.get();
    shader->bind();
    shader->bindUniform(camera.getViewMatrix(), "view");
    shader->bindUniform(camera.getProjectionMatrix(), "projection");
    shader->bindUniform(camera.getPosition(), "viewPos");

    // Bind IBL maps + assign their sampler units via the scene environment.
    scene.getEnvironment().bindIBL(*shader);
    glActiveTexture(GL_TEXTURE0); // leave unit 0 active for material binds

    // UBO binding points are context state that other passes may touch — rebind per frame
    glBindBufferBase(GL_UNIFORM_BUFFER, LIGHTS_BLOCK_BINDING, m_lights->getUBO());
    glBindBufferBase(GL_UNIFORM_BUFFER, MATERIALS_BLOCK_BINDING, m_materials->getUBO());

    auto entt_view = m_registry->view<MeshRendererComponent, WorldTransformComponent>();
    for (auto entity : entt_view) {
        const auto& transform = entt_view.get<WorldTransformComponent>(entity);
        const auto& renderer = entt_view.get<MeshRendererComponent>(entity);

        Mesh* mesh = m_assets->getMesh(renderer.mesh);
        if (!renderer.visible || !mesh) continue;

        shader->bindUniform(transform.matrix, "model");
        shader->bindUniform(static_cast<int>(renderer.material), "materialIndex");
        m_materials->bindMaterialTextures(renderer.material);
        mesh->draw();
    }
    shader->unbind();
}

}
