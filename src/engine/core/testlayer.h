#pragma once
#include "layer.h"
#include "scene/entity.h"
#include "graphics/material_system.h"
#include "graphics/render_system.h"
#include "scene/scene.h"
#include "graphics/lighting_system.h"
#include "physics/physics_system.h"
#include "graphics/spectator_camera.h"
#include "scene/cube_mesh.h"
#include "scene/plane_mesh.h"
#include "scene/sphere_mesh.h"
#include "scene/model.h"
#include <memory>
#include <vector>

namespace Karbon {

class TestLayer : public Layer {
public:
    TestLayer(Scene* scene, RenderSystem* renderSystem, PhysicsSystem* physicsSystem,
              LightingSystem* lightingSystem, MaterialSystem* materialSystem,
              SpectatorCamera* camera);TestLayer() = default;
    ~TestLayer() override;

    void onAttach() override;
    void onDetach() override;
    void onUpdate(float deltaTime) override;
    void onImGuiRender() override;

private:
    Scene* m_scene;
    RenderSystem* m_renderSystem;
    PhysicsSystem* m_physicsSystem;
    LightingSystem* m_lightingSystem;
    MaterialSystem* m_materialSystem;
    SpectatorCamera* m_camera;

    // Resources owned by this layer so they outlive the attach/update calls
    std::unique_ptr<Cubemap> m_skybox;
    std::unique_ptr<Texture> m_testTexture;
    std::unique_ptr<Texture> m_testTexture2;
    std::unique_ptr<PlaneMesh> m_floorMesh;
    std::unique_ptr<CubeMesh> m_cubeMesh;
    std::unique_ptr<SphereMesh> m_sphereMesh;
    std::unique_ptr<Model> m_monkeyModel;

    // Entities
    Entity m_floorEntity;
    Entity m_cubeEntity;
    Entity m_monkeyEntity;
    Entity m_light1;
    Entity m_light2;
    std::vector<Entity> m_sphereEntities;

    // Materials
    MaterialHandle m_floorMaterial{0};
    MaterialHandle m_sphereMaterial{0};
    MaterialHandle m_monkeyMaterial{0};

};

}