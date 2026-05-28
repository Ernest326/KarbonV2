#include "scene.h"
#include "components/transform.h"
#include "components/hierarchy_component.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

static void decomposeTransform(const glm::mat4& matrix, glm::vec3& position, glm::quat& rotation, glm::vec3& scale) {
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(matrix,scale,rotation,position,skew,perspective);
    rotation = glm::normalize(rotation);
}

namespace Karbon {

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

        unparent(entity);

        auto& hierarchy = m_registry.get<HierarchyComponent>(entity);
        for(auto child : hierarchy.children) {
            unparent(child);
            destroyEntity(child);
        }

        m_entityMap.erase(m_registry.get<IDComponent>(entity).id);
        m_registry.destroy(entity);
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
            auto& parent_hierarchy = m_registry.get<HierarchyComponent>(hierarchy.parent);
            parent_hierarchy.children.erase(std::remove(parent_hierarchy.children.begin(), parent_hierarchy.children.end(), child), parent_hierarchy.children.end());
            hierarchy.parent = entt::null;
        }
    }

    void Scene::setParent(entt::entity child, entt::entity parent) {
        if(child == parent || !m_registry.valid(child)) return;

        auto& childHierarchy = m_registry.get<HierarchyComponent>(child);

        updateWorldTransform(child, glm::mat4(1.0f)); // Force recompute of child beforehand to prevent stale transform
        auto& childWorld = m_registry.get<WorldTransformComponent>(child);
        glm::mat4 childWorldMatrix = childWorld.matrix;

        unparent(child);

        if(m_registry.valid(parent)) {
            updateWorldTransform(parent, glm::mat4(1.0f)); // Force recompute of parent beforehand to prevent stale transform
            auto& parentWorld = m_registry.get_or_emplace<WorldTransformComponent>(parent);
            glm::mat4 newLocal = glm::inverse(parentWorld.matrix) * childWorldMatrix;

            auto& childLocal = m_registry.get_or_emplace<TransformComponent>(child);
            childLocal.position = glm::vec3(newLocal[3]);

            childLocal.scale = glm::vec3(
                glm::length(glm::vec3(newLocal[0])),
                glm::length(glm::vec3(newLocal[1])),
                glm::length(glm::vec3(newLocal[2]))
            );

            glm::mat3 rotMat(
                childLocal.scale.x > 0.0001f ? glm::vec3(newLocal[0]) / childLocal.scale.x : glm::vec3(1,0,0),
                childLocal.scale.y > 0.0001f ? glm::vec3(newLocal[1]) / childLocal.scale.y : glm::vec3(0,1,0),
                childLocal.scale.z > 0.0001f ? glm::vec3(newLocal[2]) / childLocal.scale.z : glm::vec3(0,0,1)
            );
            childLocal.rotation = glm::normalize(glm::quat_cast(rotMat));

            auto& parent_hierarchy = m_registry.get<HierarchyComponent>(parent);
            parent_hierarchy.children.push_back(child);

            auto& childHierarchy = m_registry.get_or_emplace<HierarchyComponent>(child);
            childHierarchy.parent = parent;
            childHierarchy.dirty = true;

            markDirtyUpward(parent);
        
    };

    void Scene::onUpdate() {
        updateHierarchy();
    }

    void Scene::updateHierarchy() {
        auto view = m_registry.view<HierarchyComponent, WorldTransformComponent, TransformComponent>();
        for (auto entity : view) {
            auto& hierarchy = view.get<HierarchyComponent>(entity);
            if(hierarchy.parent == entt::null) {
                updateWorldTransform(entity, glm::mat4(1.0f));
            }
        }
    }

    void Scene::updateWorldTransform(entt::entity entity, const glm::mat4& parentMatrix) {

        glm::mat4 local = transform.getLocalMatrix();
        worldTransform.matrix = parentMatrix * local;
        worldTransform.worldPosition = glm::vec3(worldTransform.matrix[3]);
        worldTransform.worldScale = glm::vec3(
            glm::length(glm::vec3(worldTransform.matrix[0])),
            glm::length(glm::vec3(worldTransform.matrix[1])),
            glm::length(glm::vec3(worldTransform.matrix[2]))
        );
        glm::mat3 rotMat(
            worldTransform.worldScale.x > 0.0001f ? glm::vec3(worldTransform.matrix[0]) / worldTransform.worldScale.x : glm::vec3(1,0,0),
            worldTransform.worldScale.y > 0.0001f ? glm::vec3(worldTransform.matrix[1]) / worldTransform.worldScale.y : glm::vec3(0,1,0),
            worldTransform.worldScale.z > 0.0001f ? glm::vec3(worldTransform.matrix[2]) / worldTransform.worldScale.z : glm::vec3(0,0,1)
        );
        worldTransform.worldRotation = glm::normalize(glm::quat_cast(rotMat));

        hierarchy.dirty = false;

        for(auto child : hierarchy.children) {
            m_registry.get<HierarchyComponent>(child).dirty = true;
            updateWorldTransform(child, worldTransform.matrix);
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