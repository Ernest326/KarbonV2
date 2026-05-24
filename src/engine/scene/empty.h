#pragma once
#include "components/transform.h"

#include "entity.h"
namespace Karbon {
class Empty : public Entity {
public:
  Empty(entt::registry* registry, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale) 
      : Entity(registry->create(), registry) {
    auto &transform = AddComponent<TransformComponent>();
    transform.position = position;
    transform.rotation = rotation;
    transform.scale = scale;
  }
  virtual ~Empty() = default;
};

} // namespace Karbon