#include "scene.h"
#include "components/transform.h"
#include "components/hierarchy_component.h"

#include <algorithm>
#include <cmath>

static glm::vec3 safeDiv(const glm::vec3& numerator, const glm::vec3& denominator) {
    return glm::vec3(
        std::abs(denominator.x) > 0.000001f ? numerator.x / denominator.x : 0.0f,
        std::abs(denominator.y) > 0.000001f ? numerator.y / denominator.y : 0.0f,
        std::abs(denominator.z) > 0.000001f ? numerator.z / denominator.z : 0.0f
    );
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

        auto& childHierarchy = m_registry.get_or_emplace<HierarchyComponent>(child);
        if(childHierarchy.parent == parent) return;

        updateHierarchy(); // Ensure current world transforms are up to date
        auto& childWorld = m_registry.get<WorldTransformComponent>(child);

        unparent(child);

        auto& childLocal = m_registry.get_or_emplace<TransformComponent>(child);

        if(m_registry.valid(parent)) {
            auto& parentWorld = m_registry.get_or_emplace<WorldTransformComponent>(parent);
            glm::quat parentInvRot = glm::inverse(parentWorld.worldRotation);
            glm::vec3 invParentScale = safeDiv(glm::vec3(1.0f), parentWorld.worldScale);

            childLocal.rotation = glm::normalize(parentInvRot * childWorld.worldRotation);
            childLocal.position = parentInvRot * ((childWorld.worldPosition - parentWorld.worldPosition) * invParentScale);
            childLocal.scale = safeDiv(childWorld.worldScale, parentWorld.worldScale);

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

        worldTransform.worldRotation = glm::normalize(parentWorld.worldRotation * transform.rotation);
        worldTransform.worldScale = parentWorld.worldScale * transform.scale;

        glm::vec3 scaledLocalPos = parentWorld.worldScale * transform.position;
        worldTransform.worldPosition = parentWorld.worldPosition + (parentWorld.worldRotation * scaledLocalPos);

        worldTransform.matrix = glm::translate(glm::mat4(1.0f), worldTransform.worldPosition)
            * glm::mat4_cast(worldTransform.worldRotation)
            * glm::scale(glm::mat4(1.0f), worldTransform.worldScale);

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