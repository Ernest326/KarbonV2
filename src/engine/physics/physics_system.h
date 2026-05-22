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

    void Initialize();
    void Shutdown();
    void Update(float deltaTime);

    void SyncPhysicsToEntities();
    void SyncEntitiesToPhysics();

    entt::registry* m_Registry = nullptr;
    JPH::JobSystem* m_JobSystem = nullptr;
    JPH::PhysicsSystem* m_PhysicsSystem = nullptr;
    JPH::TempAllocatorImpl m_TempAllocator;

    std::unique_ptr<JPH::ObjectLayerPairFilter> m_ObjectLayerPairFilter;
    std::unique_ptr<JPH::BroadPhaseLayerInterface> m_BroadPhaseLayerInterface;
    std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilter> m_ObjectVsBroadPhaseLayerFilter;

    //EnTT hooks
    void OnRigidbodyAdded(entt::registry& registry, entt::entity entity);
    void OnRigidbodyRemoved(entt::registry& registry, entt::entity entity);
    void OnColliderAdded(entt::registry& registry, entt::entity entity);
    void OnColliderRemoved(entt::registry& registry, entt::entity entity);
    void OnTransformAdded(entt::registry& registry, entt::entity entity);
    void OnTransformRemoved(entt::registry& registry, entt::entity entity);

    // Helpers
    void TryCreateBody(entt::registry& registry, entt::entity entity);
    void RemoveBody(entt::entity entity);

    std::unordered_map<entt::entity, JPH::BodyID> m_EntityToBodyMap;
    std::unordered_map<JPH::BodyID, entt::entity> m_BodyToEntityMap;

    float m_Accum = 0.0f;
    const float m_FixedTimeStep = 1.0f / 60.0f; // 60 FPS

    inline int getBodyCount() const { return bodyCounter; }

private:
    int bodyCounter = 0;
};

}