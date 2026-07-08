#include "scene.h"
#include "components/transform.h"
#include "components/hierarchy_component.h"
#include "components/camera_component.h"

#include <algorithm>
#include <cmath>

static void decomposeNoShear(const glm::mat4& matrix, glm::vec3& position, glm::quat& rotation, glm::vec3& scale) {
    position = glm::vec3(matrix[3]);

    glm::vec3 col0 = glm::vec3(matrix[0]);
    glm::vec3 col1 = glm::vec3(matrix[1]);
    glm::vec3 col2 = glm::vec3(matrix[2]);

    scale.x = glm::length(col0);
    if (scale.x > 0.000001f) {
        col0 /= scale.x;
    } else {
        col0 = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    col1 -= col0 * glm::dot(col1, col0);
    scale.y = glm::length(col1);
    if (scale.y > 0.000001f) {
        col1 /= scale.y;
    } else {
        col1 = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    col2 -= col0 * glm::dot(col2, col0) + col1 * glm::dot(col2, col1);
    scale.z = glm::length(col2);
    if (scale.z > 0.000001f) {
        col2 /= scale.z;
    } else {
        col2 = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    glm::mat3 rotMat(col0, col1, col2);
    rotation = glm::normalize(glm::quat_cast(rotMat));
}

namespace Karbon {
    // Scene environment implementation
    void SceneEnvironment::setCubemap(const std::vector<std::string>& faces) {
        clear();
        m_cubemap = std::make_unique<Cubemap>(faces);
        if (m_cubemap->getID() != 0) {
            m_type = Type::Cubemap;
            m_cubemapFaces = faces;
            m_hdrPath.clear();
        } else {
            m_cubemap.reset();
        }
    }

    void SceneEnvironment::setHDR(const std::string& hdrPath) {
        clear();
        m_environmentMap = std::make_unique<EnvironmentMap>(hdrPath);
        if (m_environmentMap->getID() != 0) {
            m_type = Type::HDR;
            m_hdrPath = hdrPath;
            m_cubemapFaces.clear();
        } else {
            m_environmentMap.reset();
        }
    }

    void SceneEnvironment::clear() {
        m_cubemap.reset();
        m_environmentMap.reset();
        m_type = Type::None;
        m_cubemapFaces.clear();
        m_hdrPath.clear();
    }

    unsigned int SceneEnvironment::getSkyboxCubemap() const {
        if (m_type == Type::Cubemap && m_cubemap) return m_cubemap->getID();
        if (m_type == Type::HDR && m_environmentMap) return m_environmentMap->getCubemap();
        return 0;
    }

    bool SceneEnvironment::hasIBL() const {
        if (m_type == Type::Cubemap && m_cubemap) return m_cubemap->hasIBL();
        if (m_type == Type::HDR && m_environmentMap) return m_environmentMap->hasIBL();
        return false;
    }

    void SceneEnvironment::generateIBL() {
        if (m_type == Type::Cubemap && m_cubemap) m_cubemap->generateIBL();
        if (m_type == Type::HDR && m_environmentMap) m_environmentMap->generateIBL();
    }

    unsigned int SceneEnvironment::getIrradianceMap() const {
        if (m_type == Type::Cubemap && m_cubemap) return m_cubemap->getIrradianceMap();
        if (m_type == Type::HDR && m_environmentMap) return m_environmentMap->getIrradianceMap();
        return 0;
    }

    unsigned int SceneEnvironment::getPrefilterMap() const {
        if (m_type == Type::Cubemap && m_cubemap) return m_cubemap->getPrefilterMap();
        if (m_type == Type::HDR && m_environmentMap) return m_environmentMap->getPrefilterMap();
        return 0;
    }

    unsigned int SceneEnvironment::getBRDFLUT() const {
        if (m_type == Type::Cubemap && m_cubemap) return m_cubemap->getBRDFLUT();
        if (m_type == Type::HDR && m_environmentMap) return m_environmentMap->getBRDFLUT();
        return 0;
    }

    void SceneEnvironment::bind() {
        if (m_type == Type::Cubemap && m_cubemap) m_cubemap->bind();
        if (m_type == Type::HDR && m_environmentMap) m_environmentMap->bind();
    }

    void SceneEnvironment::unbind() {
        if (m_type == Type::Cubemap && m_cubemap) m_cubemap->unbind();
        if (m_type == Type::HDR && m_environmentMap) m_environmentMap->unbind();
    }

    void SceneEnvironment::bindIBL(Shader& shader) {
        shader.bind();

        const int IRR_SLOT = 8;
        const int PREF_SLOT = 9;
        const int BRDF_SLOT = 10;

        // FIX: Assign the sampler units UNCONDITIONALLY. When these were only
        // set inside the hasIBL() branch, the samplers defaulted to unit 0
        // whenever IBL was absent — putting samplerCube irradianceMap/
        // prefilterMap on the same unit as the sampler2D material maps.
        // A samplerCube and sampler2D sharing a unit in one program raises
        // GL_INVALID_OPERATION at draw time and the draw is silently dropped.
        shader.bindUniform(IRR_SLOT, "irradianceMap");
        shader.bindUniform(PREF_SLOT, "prefilterMap");
        shader.bindUniform(BRDF_SLOT, "brdfLUT");

        if (this->hasIBL()) {
            glActiveTexture(GL_TEXTURE0 + IRR_SLOT);
            glBindTexture(GL_TEXTURE_CUBE_MAP, getIrradianceMap());

            glActiveTexture(GL_TEXTURE0 + PREF_SLOT);
            glBindTexture(GL_TEXTURE_CUBE_MAP, getPrefilterMap());

            glActiveTexture(GL_TEXTURE0 + BRDF_SLOT);
            glBindTexture(GL_TEXTURE_2D, getBRDFLUT());

            shader.bindUniform(1, "u_HasIBL");
        } else {
            shader.bindUniform(0, "u_HasIBL");
        }
    }

    // -----------------------------------------------------

    entt::entity Scene::createEntity(const std::string& tag) {
        auto entity = m_registry.create();
        m_registry.emplace<IDComponent>(entity, m_nextUUID++);
        m_registry.emplace<TagComponent>(entity, tag);
        m_registry.emplace<TransformComponent>(entity);
        m_registry.emplace<WorldTransformComponent>(entity);
        m_registry.emplace<HierarchyComponent>(entity);
        m_entityMap[m_registry.get<IDComponent>(entity).id] = entity;

        return entity;
    }

    void Scene::destroyEntity(entt::entity entity) {
        if(!m_registry.valid(entity)) return;

        if (entity == m_primaryCamera) {
            m_primaryCamera = entt::null;
        }

        unparent(entity);

        // Copy the children: unparent()/destroyEntity() erase from the live
        // vector and can reallocate registry storage, invalidating iterators.
        auto children = m_registry.get<HierarchyComponent>(entity).children;
        for(auto child : children) {
            unparent(child);
            destroyEntity(child);
        }

        m_entityMap.erase(m_registry.get<IDComponent>(entity).id);
        m_registry.destroy(entity);
    }

    void Scene::setPrimaryCamera(entt::entity entity) {
        if (!m_registry.valid(entity) || !m_registry.all_of<CameraComponent>(entity)) {
            return;
        }

        if (m_primaryCamera != entt::null && m_registry.valid(m_primaryCamera) &&
            m_registry.all_of<CameraComponent>(m_primaryCamera)) {
            m_registry.get<CameraComponent>(m_primaryCamera).primary = false;
        }

        m_primaryCamera = entity;
        m_registry.get<CameraComponent>(entity).primary = true;
    }

    Camera* Scene::getPrimaryCamera() {
        if (m_primaryCamera != entt::null && m_registry.valid(m_primaryCamera) &&
            m_registry.all_of<CameraComponent>(m_primaryCamera)) {
            return &m_registry.get<CameraComponent>(m_primaryCamera).camera;
        }

        auto view = m_registry.view<CameraComponent>();
        for (auto entity : view) {
            auto& component = view.get<CameraComponent>(entity);
            if (component.primary) {
                m_primaryCamera = entity;
                return &component.camera;
            }
        }

        for (auto entity : view) {
            auto& component = view.get<CameraComponent>(entity);
            m_primaryCamera = entity;
            component.primary = true; // sync flag with cached handle
            return &component.camera;
        }

        return nullptr;
    }

    void Scene::renderSkybox(Shader& skyboxShader) {
        Camera* camera = getPrimaryCamera();

        if (!camera || m_sceneEnvironment.getSkyboxCubemap() == 0)
            return;

        if (m_sceneEnvironment.getSkyboxCubemap() != 0) {
            GLboolean depthMask;
            glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);

            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_FALSE);

            m_sceneEnvironment.bind();

            skyboxShader.bind();
            skyboxShader.bindUniform(camera->getProjectionMatrix(), "projection");

            glm::mat4 viewRot = glm::mat4(glm::mat3(camera->getViewMatrix()));
            skyboxShader.bindUniform(viewRot, "view");
            skyboxShader.bindUniform(0, "skybox");

            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

            m_sceneEnvironment.unbind();

            glDepthMask(depthMask);
            glDepthFunc(GL_LESS);
        }
    }

    entt::entity Scene::findByTag(const std::string& tag) {
        auto view = m_registry.view<TagComponent>();
        for (auto entity : view) {
            if(view.get<TagComponent>(entity).tag == tag) {
                return entity;
            }
        }
        return entt::null;
    }

    entt::entity Scene::findByUUID(UUID id) {
        auto it = m_entityMap.find(id);
        if(it != m_entityMap.end()) {
            return it->second;
        }
        return entt::null;
    }

    void Scene::unparent(entt::entity child) {
        if(!m_registry.valid(child)) return;

        auto& hierarchy = m_registry.get<HierarchyComponent>(child);
        if(hierarchy.parent != entt::null) {
            updateHierarchy();

            auto& parent_hierarchy = m_registry.get<HierarchyComponent>(hierarchy.parent);
            parent_hierarchy.children.erase(std::remove(parent_hierarchy.children.begin(), parent_hierarchy.children.end(), child), parent_hierarchy.children.end());
            hierarchy.parent = entt::null;
            hierarchy.dirty = true;

            auto& childWorld = m_registry.get<WorldTransformComponent>(child);
            auto& childLocal = m_registry.get<TransformComponent>(child);
            childLocal.setLocalMatrix(childWorld.matrix);
            decomposeNoShear(childWorld.matrix, childLocal.position, childLocal.rotation, childLocal.scale);

            updateWorldTransform(child, WorldTransformComponent{});
        }
    }

    void Scene::setParent(entt::entity child, entt::entity parent) {
        if(child == parent || !m_registry.valid(child)) return;

        auto& childHierarchy = m_registry.get_or_emplace<HierarchyComponent>(child);
        if(childHierarchy.parent == parent) return;

        updateHierarchy(); // Ensure current world transforms are up to date
        auto& childWorld = m_registry.get<WorldTransformComponent>(child);

        unparent(child);

        auto& childLocal = m_registry.get_or_emplace<TransformComponent>(child);

        if(m_registry.valid(parent)) {
            auto& parentWorld = m_registry.get_or_emplace<WorldTransformComponent>(parent);
            glm::mat4 localMatrix = glm::inverse(parentWorld.matrix) * childWorld.matrix;
            childLocal.setLocalMatrix(localMatrix);
            decomposeNoShear(localMatrix, childLocal.position, childLocal.rotation, childLocal.scale);

            auto& parentHierarchy = m_registry.get<HierarchyComponent>(parent);
            if(std::find(parentHierarchy.children.begin(), parentHierarchy.children.end(), child) == parentHierarchy.children.end()) {
                parentHierarchy.children.push_back(child);
            }

            childHierarchy.parent = parent;
            childHierarchy.dirty = true;

            markDirtyUpward(parent);
            updateWorldTransform(child, parentWorld);
        } else {
            childLocal.position = childWorld.worldPosition;
            childLocal.rotation = childWorld.worldRotation;
            childLocal.scale = childWorld.worldScale;

            childHierarchy.parent = entt::null;
            childHierarchy.dirty = true;
            updateWorldTransform(child, WorldTransformComponent{});
        }
    }

    void Scene::onUpdate() {
        updateHierarchy();
    }

    void Scene::updateHierarchy() {
        auto view = m_registry.view<HierarchyComponent, WorldTransformComponent, TransformComponent>();
        WorldTransformComponent rootWorld;
        for (auto entity : view) {
            auto& hierarchy = view.get<HierarchyComponent>(entity);
            if(hierarchy.parent == entt::null) {
                updateWorldTransform(entity, rootWorld);
            }
        }
    }

    void Scene::updateWorldTransform(entt::entity entity, const WorldTransformComponent& parentWorld) {
        if(!m_registry.valid(entity)) return;

        auto& transform = m_registry.get<TransformComponent>(entity);
        auto& worldTransform = m_registry.get<WorldTransformComponent>(entity);
        auto& hierarchy = m_registry.get<HierarchyComponent>(entity);

        glm::mat4 localMatrix = transform.getLocalMatrix();
        worldTransform.matrix = parentWorld.matrix * localMatrix;
        decomposeNoShear(worldTransform.matrix, worldTransform.worldPosition, worldTransform.worldRotation, worldTransform.worldScale);

        hierarchy.dirty = false;

        for(auto child : hierarchy.children) {
            if(!m_registry.valid(child)) continue;
            m_registry.get<HierarchyComponent>(child).dirty = true;
            updateWorldTransform(child, worldTransform);
        }
    }

    void Scene::markDirtyUpward(entt::entity entity) {
        while(m_registry.valid(entity)) {
            auto& hierarchy = m_registry.get<HierarchyComponent>(entity);
            if(hierarchy.dirty) break;
            hierarchy.dirty = true;
            entity = hierarchy.parent;
        }
    }
    
    void Scene::markDirtyDownward(entt::entity entity) {
        if(!m_registry.valid(entity))
            return;

        auto& hierarchy = m_registry.get<HierarchyComponent>(entity);
        hierarchy.dirty = true;

        for(auto child : hierarchy.children) {
            markDirtyDownward(child);
        }
    }

}