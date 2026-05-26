#include "application.h"
#include "../events/application_event.h"
#include "../input/inputsystem.h"
#include "base.h"
#include <iostream>

#include "../graphics/buffers/buffers.h"
#include "../graphics/shader.h"
#include "../graphics/texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../ui/kbimgui.h"

#include "../graphics/primitives/cube.h"
#include "../graphics/primitives/plane.h"
#include "../graphics/spectator_camera.h"

#include "../scene/empty.h"

// Physics incldues
#include "../physics/physics_system.h"
#include "../scene/components/collider_component.h"
#include "../scene/components/rigidbody_component.h"
#include "../scene/components/transform.h"
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/RegisterTypes.h>

//Lighting
#include "../graphics/lighting_system.h"
#include "../scene/components/pointlight_component.h"

//Rendering
#include "../graphics/render_system.h"
#include "../graphics/material_system.h"
#include "../scene/plane_mesh.h"
#include "../scene/cube_mesh.h"
#include "../scene/sphere_mesh.h"
#include "../scene/components/meshrenderer_component.h"

namespace Karbon {

namespace {
glm::quat quatFromDegrees(const glm::vec3 &degrees) {
  glm::vec3 radians = glm::radians(degrees);
  glm::quat rotationX = glm::angleAxis(radians.x, glm::vec3(1.0f, 0.0f, 0.0f));
  glm::quat rotationY = glm::angleAxis(radians.y, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::quat rotationZ = glm::angleAxis(radians.z, glm::vec3(0.0f, 0.0f, 1.0f));
  return rotationX * rotationY * rotationZ;
}
} // namespace

float deltaTime = 0.0f;
Application *Application::s_instance = nullptr;
Application &Application::Get() { return *Application::s_instance; }

Application::Application(const char *title) {
  s_instance = this;
  WindowProperties specification(title);
  m_window = std::make_unique<Window>(specification);
  m_window->setEventCallback(KB_BIND_EVENT_FN(Application::OnEvent));
}

Application::~Application() {}

void Application::run() {
  std::cout << "Running application..." << std::endl;

  // Jolt physics one-time global init
  JPH::RegisterDefaultAllocator();
  JPH::Factory::sInstance = new JPH::Factory();
  JPH::RegisterTypes();

  KarbonImGUI::init();

  // EnTT registry
  entt::registry registry;

  // Physics system
  JPH::JobSystemThreadPool jobSystem(2048, 8,
                                     std::thread::hardware_concurrency() - 1);
  PhysicsSystem physicsSystem(&registry, &jobSystem);
  physicsSystem.Initialize();

  // Renderer system
  LightingSystem lightingSystem(&registry);
  lightingSystem.Initialize();
  MaterialSystem materialSystem; 
  RenderSystem renderSystem(&registry, &materialSystem, &lightingSystem); 
  
  Cubemap skybox({
    "resources/textures/skybox/right.jpg",
    "resources/textures/skybox/left.jpg",
    "resources/textures/skybox/top.jpg",
    "resources/textures/skybox/bottom.jpg",
    "resources/textures/skybox/front.jpg",
    "resources/textures/skybox/back.jpg"
  });
  renderSystem.setSkybox(&skybox);

  // Shader + texture setup
  Shader test_shader("resources/shaders/test_standard.vert",
                     "resources/shaders/test_standard.frag");
  Texture test_texture("resources/textures/texture.png");
  Texture test_texture2("resources/textures/texture2.jpg");

  // Visual primitives
  SpectatorCamera camera(glm::vec3(0.0f, 0.0f, 5.0f),
                         glm::vec3(0.0f, 0.0f, 0.0f));

  // Entities ----------------------------
  
  MaterialHandle floor_material = materialSystem.createTextured(glm::vec4(1.0f), 0.0f, 1.0f, &test_texture);

  // Floor
  PlaneMesh floor_mesh;
  Empty floor_entity(&registry, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(20.0f, 0.1f, 20.0f));

  floor_entity.AddComponent<RigidbodyComponent>();
  floor_entity.GetComponent<RigidbodyComponent>().type = RigidbodyComponent::Type::Kinematic;
  floor_entity.GetComponent<RigidbodyComponent>().mass = 0.0f;
  floor_entity.GetComponent<RigidbodyComponent>().friction = 0.5f;

  floor_entity.AddComponent<ColliderComponent>();
  floor_entity.GetComponent<ColliderComponent>().type = ColliderComponent::Type::Plane;
  floor_entity.GetComponent<ColliderComponent>().halfExtents = glm::vec3(10.0f, 1.0f, 10.0f);

  floor_entity.AddComponent<MeshRendererComponent>();
  floor_entity.GetComponent<MeshRendererComponent>().mesh = &floor_mesh;
  floor_entity.GetComponent<MeshRendererComponent>().material = floor_material;


  //Sphere generation
  SphereMesh sphere_mesh;
  MaterialHandle sphere_material = materialSystem.create(glm::vec4(0.8f, 0.2f, 0.2f, 1.0f), 0.0f, 0.2f);

  std::vector<entt::entity> sphere_entities;
  for (int i = 0; i < 1000; i++) {
    int x = rand() % 10 - 5;
    int z = rand() % 10 - 5;

    Empty sphere_entity(&registry, glm::vec3((float)x, 15.0f + (float)i * 1.5f, (float)z), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));

    sphere_entity.AddComponent<RigidbodyComponent>();
    sphere_entity.GetComponent<RigidbodyComponent>().type = RigidbodyComponent::Type::Dynamic;
    sphere_entity.GetComponent<RigidbodyComponent>().physicsType = RigidbodyComponent::PhysicsType::Continuous;
    sphere_entity.GetComponent<RigidbodyComponent>().mass = 1.0f;
    sphere_entity.GetComponent<RigidbodyComponent>().friction = 0.5f;

    sphere_entity.AddComponent<ColliderComponent>();
    sphere_entity.GetComponent<ColliderComponent>().type = ColliderComponent::Type::Sphere;
    sphere_entity.GetComponent<ColliderComponent>().radius = 0.5f;

    sphere_entity.AddComponent<MeshRendererComponent>();
    sphere_entity.GetComponent<MeshRendererComponent>().mesh = &sphere_mesh;
    sphere_entity.GetComponent<MeshRendererComponent>().material = sphere_material;

    sphere_entities.push_back(sphere_entity.getID());
  }

  //Create a light
  Empty light(&registry, glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
  PointLightComponent &pointlight = light.AddComponent<PointLightComponent>();
  pointlight.color = glm::vec3(1.0f, 1.0f, 1.0f);
  pointlight.intensity = 2.0f;
  pointlight.radius = 5.0f;
 
  Empty light2(&registry, glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
  PointLightComponent &pointlight2 = light2.AddComponent<PointLightComponent>();
  pointlight2.color = glm::vec3(1.0f, 0.0f, 0.0f);
  pointlight2.intensity = 5.0f;
  pointlight2.radius = 15.0f;

  //---------------------------------

  // Some defaults
  float lastFrameTime = getTime();

  // GL defaults
  glEnable(GL_DEPTH_TEST);

  // Game loop
  while (m_running) {
    float currentFrameTime = getTime();
    deltaTime = currentFrameTime - lastFrameTime;
    lastFrameTime = currentFrameTime;

    InputSystem::Get().BeginFrame();
    glfwPollEvents();
    camera.update(deltaTime);
    m_window->clear();
    KarbonImGUI::begin();

    if (!m_minimised) {

      physicsSystem.Update(deltaTime);
      lightingSystem.Update();

      ImGui::Begin("Test Window");
      ImGui::Text("Hello, world!");
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::Text("Position: (%.2f, %.2f, %.2f)", camera.getPosition().x,
                  camera.getPosition().y, camera.getPosition().z);
      ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", camera.getRotation().x,
                  camera.getRotation().y, camera.getRotation().z);
      ImGui::Text("Body count: %d", physicsSystem.getBodyCount());
      ImGui::End();

      //Update floor position
      auto &floor_transform = floor_entity.GetComponent<TransformComponent>();
      floor_transform.position = glm::vec3(0.0f, sin(glfwGetTime() * 2.0f) * 2.0f - 1.0f, 0.0f);

      //Lighting UBO binding
      GLuint lights = glGetUniformBlockIndex(test_shader.getID(), "Lights");
      glUniformBlockBinding(test_shader.getID(), lights, 1);
      glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightingSystem.getUBO());

      //Update light position
      auto &light_transform = light2.GetComponent<TransformComponent>();
      light_transform.position = glm::vec3(sin(glfwGetTime() * 5.0f) * 5.0f, 2.0f, cos(glfwGetTime() * 5.0f) * 5.0f);

      renderSystem.Draw(&test_shader, camera.getViewMatrix(), camera.getProjectionMatrix(), camera.getPosition());

    }

    KarbonImGUI::end();
    m_window->update();
  }

  // Shutdown
  physicsSystem.Shutdown();
  JPH::UnregisterTypes();
  delete JPH::Factory::sInstance;
  JPH::Factory::sInstance = nullptr;

  KarbonImGUI::shutdown();
  std::cout << "Closing application..." << std::endl;
}

void Application::quit() { m_running = false; }

bool Application::OnWindowClose(WindowCloseEvent &e) {
  quit();
  return true;
}

bool Application::OnWindowResize(WindowResizeEvent &e) {
  if (e.getWidth() == 0 || e.getHeight() == 0) {
    m_minimised = true;
    return false;
  }
  m_minimised = false;
  // Renderer on resize

  return false;
}

bool Application::OnKeyPress(KeyPressEvent &e) {
  if (e.getKeyCode() == GLFW_KEY_Q)
    quit();
  if (e.getKeyCode() == GLFW_KEY_ESCAPE) {
    // Toggle mouse visible
    GLFWwindow *window = m_window->getGLWindow();
    int mode = glfwGetInputMode(window, GLFW_CURSOR);
    if (mode == GLFW_CURSOR_NORMAL) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
  }
  return true;
}

void Application::OnEvent(Event &e) {
  InputSystem::Get().OnEvent(e);
  EventDispatcher dispatcher(e);
  dispatcher.Dispatch<WindowCloseEvent>(
      KB_BIND_EVENT_FN(Application::OnWindowClose));
  dispatcher.Dispatch<WindowResizeEvent>(
      KB_BIND_EVENT_FN(Application::OnWindowResize));
  dispatcher.Dispatch<KeyPressEvent>(KB_BIND_EVENT_FN(Application::OnKeyPress));
}

float Application::getTime() { return static_cast<float>(glfwGetTime()); }

float Application::getDeltaTime() { return deltaTime; }

} // namespace Karbon