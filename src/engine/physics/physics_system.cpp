#include "physics_system.h"
#include "scene/components/collider_component.h"
#include "scene/components/hierarchy_component.h"
#include "scene/components/rigidbody_component.h"
#include "scene/components/transform.h"
#include <Jolt/Geometry/Plane.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/PlaneShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <iostream>

namespace Karbon {

struct PhysicsBodyTag {};

// Layer filters for collision
namespace {
constexpr JPH::ObjectLayer NON_MOVING = 0;
constexpr JPH::ObjectLayer MOVING = 1;
constexpr JPH::ObjectLayer NUM_LAYERS = 2;

constexpr JPH::BroadPhaseLayer BROAD_PHASE_LAYER_NON_MOVING(0);
constexpr JPH::BroadPhaseLayer BROAD_PHASE_LAYER_MOVING(1);
constexpr JPH::uint NUM_BROAD_PHASE_LAYERS = 2;

class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
public:
  virtual bool ShouldCollide(JPH::ObjectLayer layer1,
                             JPH::ObjectLayer layer2) const override {
    switch (layer1) {
    case NON_MOVING:
      return layer2 == MOVING;
    case MOVING:
      return true;
    default:
      return false;
    }
  }
};

class BroadPhaseLayerInterfaceImpl : public JPH::BroadPhaseLayerInterface {
private:
  JPH::BroadPhaseLayer mObjectToBroadPhaseLayer[NUM_LAYERS];

public:
  BroadPhaseLayerInterfaceImpl() {
    mObjectToBroadPhaseLayer[NON_MOVING] = BROAD_PHASE_LAYER_NON_MOVING;
    mObjectToBroadPhaseLayer[MOVING] = BROAD_PHASE_LAYER_MOVING;
  }
  virtual JPH::uint GetNumBroadPhaseLayers() const override {
    return NUM_BROAD_PHASE_LAYERS;
  }
  virtual JPH::BroadPhaseLayer
  GetBroadPhaseLayer(JPH::ObjectLayer objectLayer) const override {
    return mObjectToBroadPhaseLayer[objectLayer];
  }
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
  virtual const char *
  GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override {
    switch ((JPH::BroadPhaseLayer::Type)layer) {
    case (JPH::BroadPhaseLayer::Type)0:
      return "NON_MOVING";
    case (JPH::BroadPhaseLayer::Type)1:
      return "MOVING";
    default:
      return "UNKNOWN";
    }
  }
#endif
};

class ObjectVsBroadPhaseLayerFilterImpl
    : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
  virtual bool
  ShouldCollide(JPH::ObjectLayer objectLayer,
                JPH::BroadPhaseLayer broadPhaseLayer) const override {
    switch (objectLayer) {
    case NON_MOVING:
      return broadPhaseLayer == BROAD_PHASE_LAYER_MOVING;
    case MOVING:
      return true;
    default:
      return false;
    }
  }
};
} // namespace

PhysicsSystem::PhysicsSystem(entt::registry *registry,
                             JPH::JobSystem *jobSystem)
    : m_registry(registry), m_jobSystem(jobSystem),
      m_tempAllocator(10 * 1024 *
                      1024), // 10 MB temp allocator for physics updates
      m_accum(0.0f), m_fixedTimeStep(1.0f / 60.0f) {}

PhysicsSystem::~PhysicsSystem() { shutdown(); }

void PhysicsSystem::initialize() {
  m_physicsSystem = new JPH::PhysicsSystem();

  m_broadPhaseLayerInterface = std::make_unique<BroadPhaseLayerInterfaceImpl>();
  m_objectVsBroadPhaseLayerFilter =
      std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();
  m_objectLayerPairFilter = std::make_unique<ObjectLayerPairFilterImpl>();

  // Init(max_bodies, num_body_mutexes, max_body_pairs, max_contact_constraints,
  // ...)
  m_physicsSystem->Init(65536, 0, 65536, 10240, *m_broadPhaseLayerInterface,
                        *m_objectVsBroadPhaseLayerFilter,
                        *m_objectLayerPairFilter);
  m_physicsSystem->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));


  // Register EnTT hooks
  m_registry->on_destroy<RigidbodyComponent>()
      .connect<&PhysicsSystem::onRigidbodyRemoved>(this);
  m_registry->on_destroy<ColliderComponent>()
      .connect<&PhysicsSystem::onColliderRemoved>(this);
  m_registry->on_destroy<TransformComponent>()
      .connect<&PhysicsSystem::onTransformRemoved>(this);
}

void PhysicsSystem::update(float deltaTime) {

  if (!m_physicsSystem || !m_jobSystem)
    return;

  // Create pending bodies
  auto view = m_registry->view<RigidbodyComponent, ColliderComponent, TransformComponent>(entt::exclude<PhysicsBodyTag>);
  for (auto entity : view) {
    tryCreateBody(*m_registry, entity);
  }

  m_accum += deltaTime;

  while (m_accum >= m_fixedTimeStep) {
    syncEntitiesToPhysics();
    m_physicsSystem->Update(m_fixedTimeStep, 1, &m_tempAllocator, m_jobSystem);
    syncPhysicsToEntities();
    m_accum -= m_fixedTimeStep;
  }
}

void PhysicsSystem::shutdown() {
  if (!m_physicsSystem) {
    return;
  }

  if (m_registry) {
    m_registry->on_destroy<RigidbodyComponent>()
        .disconnect<&PhysicsSystem::onRigidbodyRemoved>(this);
    m_registry->on_destroy<ColliderComponent>()
        .disconnect<&PhysicsSystem::onColliderRemoved>(this);
    m_registry->on_destroy<TransformComponent>()
        .disconnect<&PhysicsSystem::onTransformRemoved>(this);
  }

  auto &bodyInterface = m_physicsSystem->GetBodyInterface();
  for (const auto &pair : m_entityToBodyMap) {
    bodyInterface.RemoveBody(pair.second);
    bodyInterface.DestroyBody(pair.second);
  }
  m_entityToBodyMap.clear();
  m_bodyToEntityMap.clear();

  delete m_physicsSystem;
  m_physicsSystem = nullptr;
}

void PhysicsSystem::onColliderRemoved(entt::registry &registry,
                                      entt::entity entity) {
  removeBody(entity);
}

void PhysicsSystem::onTransformRemoved(entt::registry &registry,
                                       entt::entity entity) {
  removeBody(entity);
}

void PhysicsSystem::tryCreateBody(entt::registry &registry,
                                  entt::entity entity) {
  // Ensure entity has all required components
  if (!registry.all_of<RigidbodyComponent, ColliderComponent,
                       TransformComponent>(entity))
    return;

  // If body already exists, do nothing
  if (m_entityToBodyMap.find(entity) != m_entityToBodyMap.end())
    return;

  auto &localTransform  = registry.get<TransformComponent>(entity);
  auto &worldTransform  = registry.get<WorldTransformComponent>(entity);
  auto &col             = registry.get<ColliderComponent>(entity);
  auto &rb              = registry.get<RigidbodyComponent>(entity);

  if (rb.isTrigger) {
    std::cout
        << "WARNING: [Physics]: Creating sensor entity for body, no collision"
        << int(entity) << std::endl;
  }

  // Set shape based on ColliderComponent
  JPH::ShapeRefC shape;
  switch (col.type) {
  case ColliderComponent::Type::Box: {
    JPH::BoxShapeSettings shape_settings(
        JPH::Vec3(col.halfExtents.x * worldTransform.worldScale.x, col.halfExtents.y * worldTransform.worldScale.y, col.halfExtents.z * worldTransform.worldScale.z));
    shape_settings.SetEmbedded();
    auto result = shape_settings.Create();
    shape = result.Get();
    break;
  }
  case ColliderComponent::Type::Sphere: {
    JPH::SphereShapeSettings shape_settings(col.radius * glm::max(worldTransform.worldScale.x, glm::max(worldTransform.worldScale.y, worldTransform.worldScale.z)));
    shape_settings.SetEmbedded();
    auto result = shape_settings.Create();
    shape = result.Get();
    break;
  }
  case ColliderComponent::Type::Capsule: {
    JPH::CapsuleShapeSettings shape_settings(col.halfExtents.y * worldTransform.worldScale.y, col.radius * glm::max(worldTransform.worldScale.x, worldTransform.worldScale.z));
    shape_settings.SetEmbedded();
    auto result = shape_settings.Create();
    shape = result.Get();
    break;
  }
  case ColliderComponent::Type::Plane: {
    JPH::BoxShapeSettings shape_settings(
        JPH::Vec3(worldTransform.worldScale.x/2, 0.05f * worldTransform.worldScale.y, worldTransform.worldScale.z/2));
    shape_settings.SetEmbedded();
    auto result = shape_settings.Create();
    shape = result.Get();
    break;
  }
  case ColliderComponent::Type::Mesh:
    // Mesh colliders would require additional handling to create a triangle
    // mesh shape
    break;
  }

  if (!shape) {
    std::cerr << "Failed to create shape for entity " << int(entity)
              << std::endl;
    return;
  }

  // Set body type
  JPH::EMotionType motionType;
  JPH::ObjectLayer objectLayer;

  switch (rb.type) {
  case RigidbodyComponent::Type::Static:
    motionType = JPH::EMotionType::Static;
    objectLayer = NON_MOVING;
    break;
  case RigidbodyComponent::Type::Kinematic:
    motionType = JPH::EMotionType::Kinematic;
    objectLayer = MOVING;
    break;
  case RigidbodyComponent::Type::Dynamic:
    motionType = JPH::EMotionType::Dynamic;
    objectLayer = MOVING;
    break;
  }

  // Finalize body creation settings
  JPH::BodyCreationSettings settings(
      shape,
      JPH::RVec3(worldTransform.worldPosition.x, worldTransform.worldPosition.y,
                 worldTransform.worldPosition.z),
      JPH::Quat(worldTransform.worldRotation.x, worldTransform.worldRotation.y,
                worldTransform.worldRotation.z, worldTransform.worldRotation.w),
      motionType, objectLayer);
  settings.mFriction = rb.friction;
  settings.mRestitution = rb.restitution;
  settings.mIsSensor = rb.isTrigger;

  if (rb.physicsType == RigidbodyComponent::PhysicsType::Continuous) {
    settings.mMotionQuality = JPH::EMotionQuality::LinearCast;
  } else {
    settings.mMotionQuality = JPH::EMotionQuality::Discrete;
  }

  if (rb.type == RigidbodyComponent::Type::Dynamic) {
    settings.mOverrideMassProperties =
        JPH::EOverrideMassProperties::CalculateInertia;
    settings.mMassPropertiesOverride.mMass = rb.mass;
  }

  // Create body and store mapping
  auto &bodyInterface = m_physicsSystem->GetBodyInterface();
  JPH::Body *body = bodyInterface.CreateBody(settings);
  if (!body) {
    std::cerr << "Failed to create physics body for entity " << int(entity)
              << std::endl;
    return;
  }

  JPH::BodyID bodyID = body->GetID();
  JPH::EActivation activation = rb.type == RigidbodyComponent::Type::Dynamic
                                    ? JPH::EActivation::Activate
                                    : JPH::EActivation::DontActivate;
  bodyInterface.AddBody(bodyID, activation);

  m_entityToBodyMap[entity] = bodyID;
  m_bodyToEntityMap[bodyID] = entity;
  registry.emplace<PhysicsBodyTag>(entity);
  m_bodyCounter++;
}

void PhysicsSystem::removeBody(entt::entity entity) {
  auto it = m_entityToBodyMap.find(entity);
  if (it != m_entityToBodyMap.end()) {
    auto &bodyInterface = m_physicsSystem->GetBodyInterface();
    bodyInterface.RemoveBody(it->second);
    bodyInterface.DestroyBody(it->second);
    m_bodyToEntityMap.erase(it->second);
    m_entityToBodyMap.erase(it);
    m_registry->erase<PhysicsBodyTag>(entity);
    m_bodyCounter--;
  }
}

void PhysicsSystem::onRigidbodyRemoved(entt::registry &registry,
                                       entt::entity entity) {
  removeBody(entity);
}

void PhysicsSystem::syncPhysicsToEntities() {
  for (const auto &pair : m_entityToBodyMap) {
    entt::entity entity = pair.first;
    JPH::BodyID bodyID = pair.second;

    JPH::RVec3 pos;
    JPH::Quat rot;
    auto &bodyInterface = m_physicsSystem->GetBodyInterface();
    bodyInterface.GetPositionAndRotation(bodyID, pos, rot);

    auto& hierarchy = m_registry->get<HierarchyComponent>(entity);
    glm::vec3 worldPos(
        static_cast<float>(pos.GetX()),
        static_cast<float>(pos.GetY()),
        static_cast<float>(pos.GetZ()));
    glm::quat worldRot(
        static_cast<float>(rot.GetW()),
        static_cast<float>(rot.GetX()),
        static_cast<float>(rot.GetY()),
        static_cast<float>(rot.GetZ()));

    auto &transform = m_registry->get<TransformComponent>(entity);

    if (hierarchy.parent != entt::null) {
        auto& parentWorld = m_registry->get<WorldTransformComponent>(hierarchy.parent);
        auto& worldTransform = m_registry->get<WorldTransformComponent>(entity);

        glm::mat4 worldMatrix =
            glm::translate(glm::mat4(1.0f), worldPos) *
            glm::mat4_cast(worldRot) *
            glm::scale(glm::mat4(1.0f), worldTransform.worldScale);

        glm::mat4 localMatrix = glm::inverse(parentWorld.matrix) * worldMatrix;

        transform.position = glm::vec3(localMatrix[3]);

        glm::mat3 rotMat(
            glm::normalize(glm::vec3(localMatrix[0])),
            glm::normalize(glm::vec3(localMatrix[1])),
            glm::normalize(glm::vec3(localMatrix[2]))
        );
        transform.rotation = glm::normalize(glm::quat_cast(rotMat));

    } else {
        transform.position = worldPos;
        transform.rotation = worldRot;
    }

    entt::entity current = entity;
    while(m_registry->get<HierarchyComponent>(current).parent != entt::null) {
        m_registry->get<HierarchyComponent>(current).dirty = true;
        current = m_registry->get<HierarchyComponent>(current).parent;
    }
  }
}

void PhysicsSystem::syncEntitiesToPhysics() {
  for (const auto &pair : m_entityToBodyMap) {
    entt::entity entity = pair.first;
    JPH::BodyID bodyID = pair.second;

    auto &rb = m_registry->get<RigidbodyComponent>(entity);
    if (rb.type != RigidbodyComponent::Type::Kinematic)
      continue;

    // Update physics body based on entity transform
    auto &transform = m_registry->get<WorldTransformComponent>(entity);
    m_physicsSystem->GetBodyInterface().MoveKinematic(
        bodyID,
        JPH::RVec3(transform.worldPosition.x, transform.worldPosition.y,
                   transform.worldPosition.z),
        JPH::Quat(transform.worldRotation.x, transform.worldRotation.y,
                  transform.worldRotation.z, transform.worldRotation.w),
        m_fixedTimeStep);
  }
}

} // namespace Karbon