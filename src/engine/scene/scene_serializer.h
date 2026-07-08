#pragma once
#include <string>

namespace Karbon {

class Scene;
class AssetManager;

// Basic scene serialization: dumps entities/components/environment to a
// simple text (.kscene) file and restores them from one.
//
// Meshes, textures, and materials are referenced by their asset path and
// re-resolved through `assets` on load — the same load-by-path/cache-by-path
// contract AssetManager already uses for loadModel/loadTexture/loadMaterial.
// A mesh renderer whose material has no backing file (created directly
// through MaterialSystem rather than AssetManager::loadMaterial) gets one
// generated under resources/materials/_generated/ the first time the scene
// is saved, so every material becomes persistable.
//
// Runtime-only physics state (Jolt body handles) is not persisted — bodies
// are recreated by PhysicsSystem from Rigidbody/Collider/Transform the next
// time it updates the entity.
bool saveScene(Scene& scene, AssetManager& assets, const std::string& path);

// Replaces the scene's current contents (via Scene::clear()) with what's in the file.
bool loadScene(Scene& scene, AssetManager& assets, const std::string& path);

}
