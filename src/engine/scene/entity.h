#pragma once
#include <entt/entt.hpp>

namespace Karbon {

class Entity {
public:
    Entity() = default;
    Entity(entt::entity handle, entt::registry* registry);
    Entity(const Entity& other) = default;

    operator entt::entity() const { return m_entityHandle; }

    template<typename T, typename... Args>
    T& addComponent(Args&&... args) {
        return m_registry->emplace<T>(m_entityHandle, std::forward<Args>(args)...);
    }

    template<typename T>
    T& getComponent() {
        return m_registry->get<T>(m_entityHandle);
    }

    template<typename T>
    bool hasComponent() {
        return m_registry->any_of<T>(m_entityHandle);
    }

private:
    entt::entity m_entityHandle{entt::null};
    entt::registry* m_registry{nullptr};
};

}