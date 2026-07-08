#include "entity.h"

namespace Karbon {

Entity::Entity(entt::entity handle, entt::registry* registry)
    : m_entityHandle(handle), m_registry(registry) {
}

} // namespace Karbon
