#include "scene.h"
#include "components/transform.h"
#include "components/hierarchy_component.h"

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