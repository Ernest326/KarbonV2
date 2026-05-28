#pragma once
#include <entt/entt.hpp>

namespace Karbon {

class Entity {
public:
    Entity() = default;
    Entity(entt::entity handle, entt::registry* registry);
    Entity(const Entity& other) = default;

    operator entt::entity() const { return m_EntityHandle; }

    template<typename T, typename... Args>
    T& AddComponent(Args&&... args) {
        return m_Registry->emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
    }

    template<typename T>
    T& GetComponent() {
        return m_Registry->get<T>(m_EntityHandle);
    }

    template<typename T>
    bool HasComponent() {
        return m_Registry->any_of<T>(m_EntityHandle);
    }

private:
    entt::entity m_EntityHandle{entt::null};
    entt::registry* m_Registry{nullptr};
};

}