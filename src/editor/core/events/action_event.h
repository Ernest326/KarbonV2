#pragma once
#include "event.h"
#include <entt/entt.hpp>
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "scene/components/transform.h"
#include "scene/components/world_transform.h"

namespace Karbon {

class ActionEvent : public Event {
public:
    virtual void undo() = 0;
};

class MoveEntityEvent : public ActionEvent {
public:
    MoveEntityEvent(entt::registry& registry, entt::entity entity, const glm::vec3& newPosition)
        : m_registry(registry), m_entity(entity), m_newPosition(newPosition) {}

    entt::entity getEntity() const { return m_entity; }
    glm::vec3 getNewPosition() const { return m_newPosition; }
    void undo() override { 
        auto& transform = m_registry.get<WorldTransform>(m_entity);
        transform.setPosition(m_oldPosition);
     }

    EVENT_CLASS_TYPE(MoveEntity);
    EVENT_CLASS_CATEGORY(EventCategoryApplication);
private:
    entt::registry& m_registry;
    entt::entity m_entity;
    glm::vec3 m_newPosition;
};

class ScaleEntityEvent : public ActionEvent {
public:
    ScaleEntityEvent(entt::registry& registry, entt::entity entity, const glm::vec3& newScale)
        : m_entity(entity), m_newScale(newScale) {}

    entt::entity getEntity() const { return m_entity; }
    glm::vec3 getNewScale() const { return m_newScale; }
    void undo() override {
        auto& transform = m_registry.get<WorldTransform>(m_entity);
        transform.setScale(m_oldScale);
    }

    EVENT_CLASS_TYPE(ScaleEntity);
    EVENT_CLASS_CATEGORY(EventCategoryApplication);
private:
    entt::registry& m_registry;
    entt::entity m_entity;
    glm::vec3 m_newScale;
};

class RotateEntityEvent : public ActionEvent {
public:
    RotateEntityEvent(entt::registry& registry, entt::entity entity, const glm::quat& newRotation)
        : m_registry(registry), m_entity(entity), m_newRotation(newRotation) {}

    entt::entity getEntity() const { return m_entity; }
    glm::quat getNewRotation() const { return m_newRotation; }
    void undo() override { 
        auto& transform = m_registry.get<WorldTransform>(m_entity);
        transform.setRotation(m_oldRotation);
    }

    EVENT_CLASS_TYPE(RotateEntity);
    EVENT_CLASS_CATEGORY(EventCategoryApplication);
private:
    entt::registry& m_registry;
    entt::entity m_entity;
    glm::quat m_newRotation;
};

class CreateEntityEvent : public ActionEvent {
public:
    CreateEntityEvent(entt::registry& registry, entt::entity entity)
        : m_registry(registry), m_entity(entity) {}

    entt::entity getEntity() const { return m_entity; }
    void undo() override { 
        m_registry.destroy(m_entity);
    }

    EVENT_CLASS_TYPE(CreateEntity);
    EVENT_CLASS_CATEGORY(EventCategoryApplication);
private:
    entt::registry& m_registry;
    entt::entity m_entity;
};

//TODO: Store hierarchy and components for undoing entity deletion
class DeleteEntityEvent : public ActionEvent {
public:
    DeleteEntityEvent(entt::registry& registry, entt::entity entity)
        : m_registry(registry), m_entity(entity) {}

    entt::entity getEntity() const { return m_entity; }
    void undo() override { 
        m_registry.create(m_entity);
    }
    EVENT_CLASS_TYPE(DeleteEntity);
    EVENT_CLASS_CATEGORY(EventCategoryApplication);
private:
    entt::registry& m_registry;
    entt::entity m_entity;
};

//TODO: Add component add/remove events
}