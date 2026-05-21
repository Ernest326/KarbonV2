#include "physics_system.h"
#include <glm/gtc/matrix_transform.hpp>
#include "../scene/components/rigidbody_component.h"
#include "../scene/components/collider_component.h"
#include "../scene/components/transform.h"

namespace Karbon {

PhysicsSystem::PhysicsSystem(entt::registry* registry, JPH::JobSystem* jobSystem) {
    m_Registry = registry;
    m_JobSystem = jobSystem;}
}

PhysicsSystem::~PhysicsSystem() {
    Shutdown();
}

void PhysicsSystem::Initialize() {
    m_PhysicsSystem = new JPH::PhysicsSystem();
    m_PhysicsSystem->Init(m_TempAllocator, m_JobSystem, 1000, 1000);

    //Register EnTT hooks
    m_Registry->on_construct<RigidbodyComponent>().connect<&PhysicsSystem::OnRigidbodyAdded>(this);
    m_Registry->on_destroy<RigidbodyComponent>().connect<&PhysicsSystem::OnRigidbodyRemoved>(this);

}

void PhysicsSystem::Update(float deltaTime) {
    m_Accum += deltaTime;
    while (m_Accum >= m_FixedTimeStep) {
        SyncEntitiesToPhysics();
        m_PhysicsSystem->Update(m_FixedTimeStep, 1, 1);
        SyncPhysicsToEntities();
        m_Accum -= m_FixedTimeStep;
    }
}

void PhysicsSystem::Shutdown() {
    if (m_PhysicsSystem) {
        delete m_PhysicsSystem;
        m_PhysicsSystem = nullptr;
    }
}

void PhysicsSystem::OnRigidbodyAdded(entt::registry& registry, entt::entity entity) {
    auto& rb = registry.get<RigidbodyComponent>(entity);
    auto& col = registry.get<ColliderComponent>(entity);

    //Create Jolt body based on RigidbodyComponent and ColliderComponent
    JPH::BodyCreationSettings settings;
    settings.mMass = rb.mass;
    settings.mFriction = rb.friction;
    settings.mRestitution = rb.restitution;
    settings.mIsSensor = rb.isTrigger;

    //Set shape based on ColliderComponent
    switch (col.type) {
        case ColliderComponent::ShapeType::Box:
            settings.SetShape(JPH::BoxShapeSettings(col.halfExtents.x, col.halfExtents.y, col.halfExtents.z));
            break;
        case ColliderComponent::ShapeType::Sphere:
            settings.SetShape(JPH::SphereShapeSettings(col.radius));
            break;
        case ColliderComponent::ShapeType::Capsule:
            settings.SetShape(JPH::CapsuleShapeSettings(col.radius, col.halfExtents.y));
            break;
        case ColliderComponent::ShapeType::Mesh:
            //Mesh colliders would require additional handling to create a triangle mesh shape
            break;
    }

    //Set body type
    switch (rb.type) {
        case RigidBodyComponent::Type::Static:
            settings.mMotionType = JPH::EMotionType::Static;
            break;
        case RigidBodyComponent::Type::Kinematic:
            settings.mMotionType = JPH::EMotionType::Kinematic;
            break;
        case RigidBodyComponent::Type::Dynamic:
            settings.mMotionType = JPH::EMotionType::Dynamic;
            break;
    }

    //Create body and store mapping
    JPH::Body* body = m_PhysicsSystem->CreateBody(settings);
    m_EntityToBodyMap[entity] = body->GetID();
}

void PhysicsSystem::OnRigidbodyRemoved(entt::registry& registry, entt::entity entity) {
    auto it = m_EntityToBodyMap.find(entity);
    if (it != m_EntityToBodyMap.end()) {
        m_PhysicsSystem->RemoveBody(it->second);
        m_EntityToBodyMap.erase(it);
    }
}

void PhysicsSystem::SyncPhysicsToEntities() {
    for (const auto& pair : m_EntityToBodyMap) {
        entt::entity entity = pair.first;
        JPH::BodyID bodyID = pair.second;

        if (m_PhysicsSystem->IsActive(bodyID)) {
            JPH::Body* body = m_PhysicsSystem->GetBody(bodyID);
            auto& rb = m_Registry->get<RigidbodyComponent>(entity);

            //Update entity transform based on physics body
            auto& transform = m_Registry->get<TransformComponent>(entity);
            transform.position = body->GetPosition();
            transform.rotation = body->GetRotation();
        }
    }
}

void PhysicsSystem::SyncEntitiesToPhysics() {
    for (const auto& pair : m_EntityToBodyMap) {
        entt::entity entity = pair.first;
        JPH::BodyID bodyID = pair.second;

        if (m_PhysicsSystem->IsActive(bodyID)) {
            JPH::Body* body = m_PhysicsSystem->GetBody(bodyID);
            auto& rb = m_Registry->get<RigidbodyComponent>(entity);

            //Update physics body based on entity transform
            //This would require a TransformComponent to be implemented to store position/rotation
            body->SetPositionAndRotation(transform.position, transform.rotation);
        }
    }
}

}