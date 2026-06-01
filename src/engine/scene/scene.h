#pragma once
#include <entt/entt.hpp>
#include <unordered_map>
#include <string>
#include "components/camera_component.h"
#include <memory>
#include <glm/glm.hpp>
#include "components/id_component.h"
#include "components/transform.h"

namespace Karbon {

class Scene {
public:
    Scene() = default;
    ~Scene() = default;

    entt::entity createEntity(const std::string& tag="Entity");
    void destroyEntity(entt::entity entity);
    entt::entity findByTag(const std::string& tag);
    entt::entity findByUUID(UUID id);

    void unparent(entt::entity child);
    void setParent(entt::entity child, entt::entity parent);

    void setPrimaryCamera(entt::entity entity);
    Camera* getPrimaryCamera();
    entt::entity getPrimaryCameraEntity() const { return m_primaryCamera; }
    entt::registry& getRegistry() { return m_registry; }

    void onUpdate();

    void markDirtyUpward(entt::entity entity);
    void markDirtyDownward(entt::entity entity);

private:
    entt::registry m_registry;
    std::unordered_map<UUID, entt::entity> m_entityMap;
    std::unordered_map<std::string, entt::entity> m_tagMap;
    UUID m_nextUUID = 1;
    entt::entity m_primaryCamera = entt::null;

    void updateHierarchy();
    void updateWorldTransform(entt::entity entity, const WorldTransformComponent& parentWorld);

    void serialize(const std::string& filepath); //TODO for future
    void deserialize(const std::string& filepath); //TODO for future

};

}