#pragma once
#include <Jolt/Physics/Body/BodyID.h>

namespace Karbon {

struct RigidbodyComponent {
    enum class Type { Static, Kinematic, Dynamic } type = Type::Static;
    float mass = 1.0f;
    float friction = 0.5f;
    float restitution = 0.0f;
    bool isTrigger = false;
    JPH::BodyID bodyID; // Store the associated Jolt BodyID
    bool initialized = false; // Flag to check if the physics body has been created
};

}