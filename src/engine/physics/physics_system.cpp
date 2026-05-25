#include "physics_system.h"
#include "../scene/components/collider_component.h"
#include "../scene/components/rigidbody_component.h"
#include "../scene/components/transform.h"
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
    : m_Registry(registry), m_JobSystem(jobSystem),
      m_TempAllocator(10 * 1024 *
                      1024), // 10 MB temp allocator for physics updates
      m_Accum(0.0f), m_FixedTimeStep(1.0f / 60.0f) {}

PhysicsSystem::~PhysicsSystem() { Shutdown(); }

void PhysicsSystem::Initialize() {
  m_PhysicsSystem = new JPH::PhysicsSystem();

  m_BroadPhaseLayerInterface = std::make_unique<BroadPhaseLayerInterfaceImpl>();
  m_ObjectVsBroadPhaseLayerFilter =
      std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();
  m_ObjectLayerPairFilter = std::make_unique<ObjectLayerPairFilterImpl>();

  // Init(max_bodies, num_body_mutexes, max_body_pairs, max_contact_constraints,
  // ...)
  m_PhysicsSystem->Init(65536, 0, 65536, 10240, *m_BroadPhaseLayerInterface,
                        *m_ObjectVsBroadPhaseLayerFilter,
                        *m_ObjectLayerPairFilter);
  m_PhysicsSystem->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));


  // Register EnTT hooks
  m_Registry->on_destroy<RigidbodyComponent>()
      .connect<&PhysicsSystem::OnRigidbodyRemoved>(this);
  m_Registry->on_destroy<ColliderComponent>()
      .connect<&PhysicsSystem::OnColliderRemoved>(this);
  m_Registry->on_destroy<TransformComponent>()
      .connect<&PhysicsSystem::OnTransformRemoved>(this);
}

void PhysicsSystem::Update(float deltaTime) {

  if (!m_PhysicsSystem || !m_JobSystem)
    return;

  // Create pending bodies
  auto view = m_Registry->view<RigidbodyComponent, ColliderComponent, TransformComponent>(entt::exclude<PhysicsBodyTag>);
  for (auto entity : view) {
    TryCreateBody(*m_Registry, entity);
  }

  m_Accum += deltaTime;

  while (m_Accum >= m_FixedTimeStep) {
    SyncEntitiesToPhysics();
    m_PhysicsSystem->Update(m_FixedTimeStep, 1, &m_TempAllocator, m_JobSystem);
    SyncPhysicsToEntities();
    m_Accum -= m_FixedTimeStep;
  }
}

void PhysicsSystem::Shutdown() {
  if (!m_PhysicsSystem) {
    return;
  }

  if (m_Registry) {
    m_Registry->on_destroy<RigidbodyComponent>()
        .disconnect<&PhysicsSystem::OnRigidbodyRemoved>(this);
    m_Registry->on_destroy<ColliderComponent>()
        .disconnect<&PhysicsSystem::OnColliderRemoved>(this);
    m_Registry->on_destroy<TransformComponent>()
        .disconnect<&PhysicsSystem::OnTransformRemoved>(this);
  }

  auto &bodyInterface = m_PhysicsSystem->GetBodyInterface();
  for (const auto &pair : m_EntityToBodyMap) {
    bodyInterface.RemoveBody(pair.second);
    bodyInterface.DestroyBody(pair.second);
  }
  m_EntityToBodyMap.clear();
  m_BodyToEntityMap.clear();

  delete m_PhysicsSystem;
  m_PhysicsSystem = nullptr;
}

void PhysicsSystem::OnColliderRemoved(entt::registry &registry,
                                      entt::entity entity) {
  RemoveBody(entity);
}

void PhysicsSystem::OnTransformRemoved(entt::registry &registry,
                                       entt::entity entity) {
  RemoveBody(entity);
}

void PhysicsSystem::TryCreateBody(entt::registry &registry,
                                  entt::entity entity) {
  // Ensure entity has all required components
  if (!registry.all_of<RigidbodyComponent, ColliderComponent,
                       TransformComponent>(entity))
    return;

  // If body already exists, do nothing
  if (m_EntityToBodyMap.find(entity) != m_EntityToBodyMap.end())
    return;

  auto &rb = registry.get<RigidbodyComponent>(entity);
  auto &col = registry.get<ColliderComponent>(entity);
  auto &transform = registry.get<TransformComponent>(entity);

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
        JPH::Vec3(col.halfExtents.x, col.halfExtents.y, col.halfExtents.z));
    shape_settings.SetEmbedded();
    auto result = shape_settings.Create();
    shape = result.Get();
    break;
  }
  case ColliderComponent::Type::Sphere: {
    JPH::SphereShapeSettings shape_settings(col.radius);
    shape_settings.SetEmbedded();
    auto result = shape_settings.Create();
    shape = result.Get();
    break;
  }
  case ColliderComponent::Type::Capsule: {
    JPH::CapsuleShapeSettings shape_settings(col.halfExtents.y, col.radius);
    shape_settings.SetEmbedded();
    auto result = shape_settings.Create();
    shape = result.Get();
    break;
  }
  case ColliderComponent::Type::Plane: {
    JPH::BoxShapeSettings shape_settings(
        JPH::Vec3(transform.scale.x/2, 0.05f, transform.scale.z/2));
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
      JPH::RVec3(transform.position.x, transform.position.y,
                 transform.position.z),
      JPH::Quat(transform.rotation.x, transform.rotation.y,
                transform.rotation.z, transform.rotation.w),
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
  auto &bodyInterface = m_PhysicsSystem->GetBodyInterface();
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

  m_EntityToBodyMap[entity] = bodyID;
  m_BodyToEntityMap[bodyID] = entity;
  registry.emplace<PhysicsBodyTag>(entity);
  bodyCounter++;
}

void PhysicsSystem::RemoveBody(entt::entity entity) {
  auto it = m_EntityToBodyMap.find(entity);
  if (it != m_EntityToBodyMap.end()) {
    auto &bodyInterface = m_PhysicsSystem->GetBodyInterface();
    bodyInterface.RemoveBody(it->second);
    bodyInterface.DestroyBody(it->second);
    m_BodyToEntityMap.erase(it->second);
    m_EntityToBodyMap.erase(it);
    m_Registry->erase<PhysicsBodyTag>(entity);
    bodyCounter--;
  }
}

void PhysicsSystem::OnRigidbodyRemoved(entt::registry &registry,
                                       entt::entity entity) {
  RemoveBody(entity);
}

void PhysicsSystem::SyncPhysicsToEntities() {
  for (const auto &pair : m_EntityToBodyMap) {
    entt::entity entity = pair.first;
    JPH::BodyID bodyID = pair.second;

    JPH::RVec3 pos;
    JPH::Quat rot;
    auto &bodyInterface = m_PhysicsSystem->GetBodyInterface();
    bodyInterface.GetPositionAndRotation(bodyID, pos, rot);

    // Update entity transform based on physics body
    auto &transform = m_Registry->get<TransformComponent>(entity);
    transform.position = glm::vec3(static_cast<float>(pos.GetX()),
                                   static_cast<float>(pos.GetY()),
                                   static_cast<float>(pos.GetZ()));
    transform.rotation = glm::quat(
        static_cast<float>(rot.GetW()), static_cast<float>(rot.GetX()),
        static_cast<float>(rot.GetY()), static_cast<float>(rot.GetZ()));
  }
}

void PhysicsSystem::SyncEntitiesToPhysics() {
  for (const auto &pair : m_EntityToBodyMap) {
    entt::entity entity = pair.first;
    JPH::BodyID bodyID = pair.second;

    auto &rb = m_Registry->get<RigidbodyComponent>(entity);
    if (rb.type != RigidbodyComponent::Type::Kinematic)
      continue;

    // Update physics body based on entity transform
    auto &transform = m_Registry->get<TransformComponent>(entity);
    m_PhysicsSystem->GetBodyInterface().MoveKinematic(
        bodyID,
        JPH::RVec3(transform.position.x, transform.position.y,
                   transform.position.z),
        JPH::Quat(transform.rotation.x, transform.rotation.y,
                  transform.rotation.z, transform.rotation.w),
        m_FixedTimeStep);
  }
}

} // namespace Karbon