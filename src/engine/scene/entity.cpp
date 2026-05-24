#include "entity.h"

namespace Karbon {

Entity::Entity(entt::entity handle, entt::registry* registry)
    : m_EntityHandle(handle), m_Registry(registry) {
}

} // namespace Karbon
