#pragma once
#include "components/transform.h"

#include "entity.h"
namespace Karbon {
class Empty : public Entity {
public:
  Empty(entt::registry* registry, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale) 
      : Entity(registry->create(), registry) {
    m_id = registry->create();
    auto &transform = AddComponent<TransformComponent>();
    transform.position = position;
    transform.rotation = rotation;
    transform.scale = scale;
  }
  virtual ~Empty() = default;
  entt::entity getID() const { return m_id; }
private:
  entt::entity m_id;
};

} // namespace Karbon