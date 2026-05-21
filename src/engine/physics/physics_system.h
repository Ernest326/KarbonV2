#pragma once
#include "../scene/entity.h"
#include <Jolt/Physics/PhysicsSystem.h>

namespace Karbon {

class PhysicsSystem {
public:
    PhysicsSystem(entt::registry* registry, JPH::JobSystem* jobSystem);
    ~PhysicsSystem();

    void Initialize();
    void Shutdown();
    void Update(float deltaTime);

private:
    entt::registry* m_Registry{nullptr};
    JPH::PhysicsSystem* m_PhysicsSystem{nullptr};
    JPH::TempAllocator m_TempAllocator(10 * 1024 * 1024); // 10 MB
    JPH::JobSystem* m_JobSystem{nullptr};

    std::unordered_map<entt::entity, JPH::BodyID> m_EntityToBodyMap;

    float m_Accum = 0.0f;
    const float m_FixedTimeStep = 1.0f / 60.0f; // 60 FPS

    //EnTT hooks
    void OnRigidbodyAdded(entt::registry& registry, entt::entity entity);
    void OnRigidbodyRemoved(entt::registry& registry, entt::entity entity);

    //Sync functions
    void SyncPhysicsToEntities();
    void SyncEntitiesToPhysics();

};

}