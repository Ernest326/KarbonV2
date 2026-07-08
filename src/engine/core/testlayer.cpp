#include "testlayer.h"
#include <glfw/glfw3.h>
#include "scene/components/transform.h"
#include "scene/components/meshrenderer_component.h"
#include "scene/components/rigidbody_component.h"
#include "scene/components/collider_component.h"
#include "scene/components/pointlight_component.h"
#include "graphics/lighting_system.h"
#include <imgui.h>

namespace Karbon {

namespace {
    glm::quat quatFromDegrees(const glm::vec3 &degrees) {
        glm::vec3 radians = glm::radians(degrees);
        glm::quat rotationX = glm::angleAxis(radians.x, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat rotationY = glm::angleAxis(radians.y, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::quat rotationZ = glm::angleAxis(radians.z, glm::vec3(0.0f, 0.0f, 1.0f));
        return rotationX * rotationY * rotationZ;
    }
}

TestLayer::TestLayer(Scene* scene, RenderSystem* renderSystem, PhysicsSystem* physicsSystem,
                     LightingSystem* lightingSystem, MaterialSystem* materialSystem,
                     SpectatorCamera* camera)
    : m_scene(scene), m_renderSystem(renderSystem), m_physicsSystem(physicsSystem),
      m_lightingSystem(lightingSystem), m_materialSystem(materialSystem), m_camera(camera) {
}

TestLayer::~TestLayer() = default;

void TestLayer::onAttach() {
    if (!m_scene || !m_renderSystem || !m_physicsSystem || !m_materialSystem) return;

    auto createEntity = [&](const std::string& name, glm::vec3 pos,
                            glm::vec3 rot = glm::vec3(0.0f),
                            glm::vec3 scale = glm::vec3(1.0f)) {
        entt::entity id = m_scene->createEntity(name);
        Entity e(id, &m_scene->getRegistry());
        auto& transform = e.getComponent<TransformComponent>();
        transform.position = pos;
        transform.rotation = quatFromDegrees(rot);
        transform.scale = scale;
        return e;
    };

    auto& env = m_scene->getEnvironment();
    env.setCubemap(std::vector<std::string>{
        "resources/textures/skybox/right.jpg",
        "resources/textures/skybox/left.jpg",
        "resources/textures/skybox/top.jpg",
        "resources/textures/skybox/bottom.jpg",
        "resources/textures/skybox/front.jpg",
        "resources/textures/skybox/back.jpg"
    });

    // Textures
    m_testTexture  = std::make_unique<Texture>("resources/textures/texture.png");
    m_testTexture2 = std::make_unique<Texture>("resources/textures/texture2.jpg");

    // Materials
    m_floorMaterial  = m_materialSystem->createTextured(glm::vec4(1.0f), 0.0f, 1.0f, m_testTexture.get());
    m_sphereMaterial = m_materialSystem->create(glm::vec4(0.8f, 0.2f, 0.2f, 1.0f), 0.0f, 0.2f);
    m_monkeyMaterial = m_materialSystem->create(glm::vec4(0.2f, 0.2f, 0.8f, 1.0f), 0.5f, 0.1f);

    // Floor
    m_floorMesh = std::make_unique<PlaneMesh>();
    m_floorEntity = createEntity("Floor", glm::vec3(0.0f, -1.0f, 0.0f),
                                 glm::vec3(0.0f), glm::vec3(20.0f, 0.1f, 20.0f));
    m_floorEntity.addComponent<RigidbodyComponent>();
    m_floorEntity.getComponent<RigidbodyComponent>().type = RigidbodyComponent::Type::Kinematic;
    m_floorEntity.getComponent<RigidbodyComponent>().mass = 0.0f;
    m_floorEntity.getComponent<RigidbodyComponent>().friction = 0.5f;
    m_floorEntity.addComponent<ColliderComponent>();
    m_floorEntity.getComponent<ColliderComponent>().type = ColliderComponent::Type::Plane;
    m_floorEntity.getComponent<ColliderComponent>().halfExtents = glm::vec3(10.0f, 1.0f, 10.0f);
    m_floorEntity.addComponent<MeshRendererComponent>();
    m_floorEntity.getComponent<MeshRendererComponent>().mesh = m_floorMesh.get();
    m_floorEntity.getComponent<MeshRendererComponent>().material = m_floorMaterial;

    // Cube
    m_cubeMesh = std::make_unique<CubeMesh>();
    m_cubeEntity = createEntity("Cube", glm::vec3(0.0f, 10.0f, 0.0f),
                                glm::vec3(45.0f, 45.0f, 60.0f), glm::vec3(1.5f, 0.1f, 2.5f));
    m_cubeEntity.addComponent<MeshRendererComponent>();
    m_cubeEntity.getComponent<MeshRendererComponent>().mesh = m_cubeMesh.get();
    m_cubeEntity.getComponent<MeshRendererComponent>().material = m_floorMaterial;
    m_cubeEntity.addComponent<RigidbodyComponent>();
    m_cubeEntity.getComponent<RigidbodyComponent>().type = RigidbodyComponent::Type::Kinematic;
    m_cubeEntity.getComponent<RigidbodyComponent>().mass = 1.0f;
    m_cubeEntity.getComponent<RigidbodyComponent>().friction = 0.5f;
    m_cubeEntity.addComponent<ColliderComponent>();
    m_cubeEntity.getComponent<ColliderComponent>().type = ColliderComponent::Type::Box;
    m_cubeEntity.getComponent<ColliderComponent>().halfExtents = glm::vec3(1.0f, 1.0f, 1.0f);

    // Parenting stress-test
    m_scene->setParent(m_cubeEntity, m_floorEntity);
    m_scene->unparent(m_cubeEntity);
    m_scene->setParent(m_cubeEntity, m_floorEntity);

    // Spheres
    m_sphereMesh = std::make_unique<SphereMesh>();
    for (int i = 0; i < 1000; i++) {
        int x = rand() % 10 - 5;
        int z = rand() % 10 - 5;
        Entity sphere = createEntity("Sphere", glm::vec3((float)x, 15.0f + (float)i * 1.5f, (float)z));

        sphere.addComponent<RigidbodyComponent>();
        sphere.getComponent<RigidbodyComponent>().type = RigidbodyComponent::Type::Dynamic;
        sphere.getComponent<RigidbodyComponent>().physicsType = RigidbodyComponent::PhysicsType::Continuous;
        sphere.getComponent<RigidbodyComponent>().mass = 1.0f;
        sphere.getComponent<RigidbodyComponent>().friction = 0.5f;

        sphere.addComponent<ColliderComponent>();
        sphere.getComponent<ColliderComponent>().type = ColliderComponent::Type::Sphere;
        sphere.getComponent<ColliderComponent>().radius = 0.5f;

        sphere.addComponent<MeshRendererComponent>();
        sphere.getComponent<MeshRendererComponent>().mesh = m_sphereMesh.get();
        sphere.getComponent<MeshRendererComponent>().material = m_sphereMaterial;

        m_sphereEntities.push_back(sphere);
    }

    // Monkey model
    m_monkeyModel = std::make_unique<Model>("resources/models/monke.fbx", m_materialSystem);
    m_monkeyEntity = createEntity("Monkey", glm::vec3(0.0f, 3.0f, 0.0f));
    m_monkeyEntity.addComponent<MeshRendererComponent>();
    if (!m_monkeyModel->getMeshes().empty()) {
        m_monkeyEntity.getComponent<MeshRendererComponent>().mesh = &m_monkeyModel->getMesh(0);
        m_monkeyEntity.getComponent<MeshRendererComponent>().material = m_monkeyMaterial;
    }

    // Lights
    m_light1 = createEntity("Light1", glm::vec3(0.0f, 2.0f, 0.0f));
    auto& pl1 = m_light1.addComponent<PointLightComponent>();
    pl1.color = glm::vec3(1.0f, 1.0f, 1.0f);
    pl1.intensity = 2.0f;
    pl1.radius = 5.0f;

    m_light2 = createEntity("Light2", glm::vec3(0.0f, 20.0f, 0.0f));
    auto& pl2 = m_light2.addComponent<PointLightComponent>();
    pl2.color = glm::vec3(1.0f, 0.0f, 0.0f);
    pl2.intensity = 5.0f;
    pl2.radius = 15.0f;

    // One-off scene update to initialise transforms
    m_scene->onUpdate();
}

void TestLayer::onDetach() {
    m_sphereEntities.clear();
}

void TestLayer::onUpdate(float deltaTime) {
    auto& floorTransform = m_floorEntity.getComponent<TransformComponent>();
    floorTransform.position = glm::vec3(0.0f, sin(glfwGetTime() * 2.0f) * 2.0f - 1.0f, 0.0f);

    auto& lightTransform = m_light2.getComponent<TransformComponent>();
    lightTransform.position = glm::vec3(sin(glfwGetTime() * 5.0f) * 5.0f, 2.0f,
                                        cos(glfwGetTime() * 5.0f) * 5.0f);
}

void TestLayer::onImGuiRender() {
    ImGui::Begin("Test Window");
    ImGui::Text("Hello, world!");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    if (m_camera) {
        ImGui::Text("Position: (%.2f, %.2f, %.2f)",
                    m_camera->getPosition().x, m_camera->getPosition().y, m_camera->getPosition().z);
        ImGui::Text("Rotation: (%.2f, %.2f, %.2f)",
                    m_camera->getRotation().x, m_camera->getRotation().y, m_camera->getRotation().z);
    }

    if (m_physicsSystem)
        ImGui::Text("Body count: %d", m_physicsSystem->getBodyCount());

    ImGui::End();
}

}