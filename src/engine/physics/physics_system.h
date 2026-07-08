#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Core/TempAllocator.h>
#include <entt/entt.hpp>
#include <unordered_map>
#include <memory>

namespace Karbon {

class PhysicsSystem {
public:
    PhysicsSystem(entt::registry* registry, JPH::JobSystem* jobSystem);
    ~PhysicsSystem();

    // Explicit lifecycle: Jolt global init/teardown (Factory, RegisterTypes)
    // is owned by Application, so this system can't be pure RAII.
    void initialize();
    void shutdown();
    void update(float deltaTime);

    int getBodyCount() const { return m_bodyCounter; }

private:
    void syncPhysicsToEntities();
    void syncEntitiesToPhysics();

    //EnTT hooks
    void onRigidbodyRemoved(entt::registry& registry, entt::entity entity);
    void onColliderRemoved(entt::registry& registry, entt::entity entity);
    void onTransformRemoved(entt::registry& registry, entt::entity entity);

    // Helpers
    void tryCreateBody(entt::registry& registry, entt::entity entity);
    void removeBody(entt::entity entity);

    entt::registry* m_registry = nullptr;
    JPH::JobSystem* m_jobSystem = nullptr;
    JPH::PhysicsSystem* m_physicsSystem = nullptr;
    JPH::TempAllocatorImpl m_tempAllocator;

    std::unique_ptr<JPH::ObjectLayerPairFilter> m_objectLayerPairFilter;
    std::unique_ptr<JPH::BroadPhaseLayerInterface> m_broadPhaseLayerInterface;
    std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilter> m_objectVsBroadPhaseLayerFilter;

    std::unordered_map<entt::entity, JPH::BodyID> m_entityToBodyMap;
    std::unordered_map<JPH::BodyID, entt::entity> m_bodyToEntityMap;

    float m_accum = 0.0f;
    const float m_fixedTimeStep = 1.0f / 60.0f; // 60 FPS

    int m_bodyCounter = 0;
};

}
