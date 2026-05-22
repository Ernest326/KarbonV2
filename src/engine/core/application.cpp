#include "application.h"
#include "base.h"
#include <iostream>
#include "../events/application_event.h"
#include "../input/inputsystem.h"

#include "../graphics/shader.h"
#include "../graphics/buffers/buffers.h"
#include "../graphics/texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../ui/kbimgui.h"

#include "../graphics/spectator_camera.h"
#include "../graphics/primitives/cube.h"
#include "../graphics/primitives/plane.h"

//Physics incldues
#include "../physics/physics_system.h"
#include "../scene/components/rigidbody_component.h"
#include "../scene/components/collider_component.h"
#include "../scene/components/transform.h"
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>

namespace Karbon {

    namespace {
        glm::quat quatFromDegrees(const glm::vec3& degrees) {
            glm::vec3 radians = glm::radians(degrees);
            glm::quat rotationX = glm::angleAxis(radians.x, glm::vec3(1.0f, 0.0f, 0.0f));
            glm::quat rotationY = glm::angleAxis(radians.y, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat rotationZ = glm::angleAxis(radians.z, glm::vec3(0.0f, 0.0f, 1.0f));
            return rotationX * rotationY * rotationZ;
        }
    }

    float deltaTime = 0.0f;
    Application* Application::s_instance = nullptr;
    Application& Application::Get() { return *Application::s_instance; }

    Application::Application(const char* title) {
        s_instance = this;
        WindowProperties specification(title);
        m_window = std::make_unique<Window>(specification);
        m_window->setEventCallback(KB_BIND_EVENT_FN(Application::OnEvent));
    }

    Application::~Application() {}

    void Application::run() {
        std::cout << "Running application..." << std::endl;

        //Jolt physics one-time global init
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

        KarbonImGUI::init();

        //EnTT + Physics registry
        entt::registry registry;
        JPH::JobSystemThreadPool jobSystem(2048, 8, std::thread::hardware_concurrency()-1);
        PhysicsSystem physicsSystem(&registry, &jobSystem);
        physicsSystem.Initialize();

        //Shader + texture setup
        Shader test_shader("resources/test_texture.vert", "resources/test_texture.frag");
        Texture test_texture("resources/texture.png");
        Texture test_texture2("resources/texture2.jpg");

        //Visual primitives
        Cube test_cube(glm::vec3(0.0f), quatFromDegrees(glm::vec3(rand()*360.0, rand()*360.0, rand()*360.0)), glm::vec3(1.0f));
        Plane test_plane(glm::vec3(0.0f, -1.0f, 0.0f), quatFromDegrees(glm::vec3(-90.0f, 0.0f, 0.0f)), glm::vec3(10.0f));
        SpectatorCamera camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f));

        //Entities with physics ----------------------------

        //Floor
        auto floor_entity = registry.create();
        auto& floor_t = registry.emplace<TransformComponent>(floor_entity);
        floor_t.position = glm::vec3(0.0f, -1.0f, 0.0f);
        floor_t.rotation = quatFromDegrees(glm::vec3(-90.0f, 0.0f, 0.0f));
        floor_t.scale = glm::vec3(10.0f);

        auto& floor_rb = registry.emplace<RigidbodyComponent>(floor_entity);
        floor_rb.type = RigidbodyComponent::Type::Static;
        floor_rb.mass = 0.0f;
        floor_rb.friction = 0.5f;

        auto& floor_col = registry.emplace<ColliderComponent>(floor_entity);
        floor_col.type = ColliderComponent::Type::Box;
        floor_col.halfExtents = glm::vec3(5.0f, 0.1f, 5.0f);

        //Cube
        auto cube_entity = registry.create();
        auto& cube_t = registry.emplace<TransformComponent>(cube_entity);
        cube_t.position = glm::vec3(0.0f, 5.0f, 0.0f);
        cube_t.rotation = quatFromDegrees(glm::vec3(0.0f));
        cube_t.scale = glm::vec3(1.0f);

        auto& cube_rb = registry.emplace<RigidbodyComponent>(cube_entity);
        cube_rb.type = RigidbodyComponent::Type::Dynamic;
        cube_rb.mass = 1.0f;
        cube_rb.friction = 0.5f;

        auto& cube_col = registry.emplace<ColliderComponent>(cube_entity);
        cube_col.type = ColliderComponent::Type::Box;
        cube_col.halfExtents = glm::vec3(0.5f);

        //Some defaults
        float lastFrameTime = getTime();

        //GL defaults
        glEnable(GL_DEPTH_TEST);

        //Game loop
        while(m_running) {
            float currentFrameTime = getTime();
            deltaTime = currentFrameTime - lastFrameTime;
            lastFrameTime = currentFrameTime;
 
            InputSystem::Get().BeginFrame();
            glfwPollEvents();
            camera.update(deltaTime);
            m_window->clear();
            KarbonImGUI::begin();

            if(!m_minimised) {

                physicsSystem.Update(deltaTime);

                ImGui::Begin("Test Window");
                ImGui::Text("Hello, world!");
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::Text("Position: (%.2f, %.2f, %.2f)", camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
                ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", camera.getRotation().x, camera.getRotation().y, camera.getRotation().z);
                ImGui::End();

                auto& floor_transform = registry.get<TransformComponent>(floor_entity);
                test_plane.setPosition(floor_transform.position);
                test_plane.setRotation(floor_transform.rotation);
                test_plane.setScale(floor_transform.scale);

                auto& cube_transform = registry.get<TransformComponent>(cube_entity);
                test_cube.setPosition(cube_transform.position);
                test_cube.setRotation(cube_transform.rotation);
                test_cube.setScale(cube_transform.scale);

                test_shader.bind();
                test_shader.bindUniform(test_cube.getModelMatrix(), "model");
                test_shader.bindUniform(camera.getViewMatrix(), "view");
                test_shader.bindUniform(camera.getProjectionMatrix(), "projection");
                test_texture.bind(0);
                test_shader.bind();
                test_cube.draw();
                test_shader.bindUniform(test_plane.getModelMatrix(), "model");
                test_texture2.bind(0);
                test_plane.draw();
                test_shader.unbind();
                test_texture.unbind();

            }

            KarbonImGUI::end();
            m_window->update();
 
        }

        //Shutdown
        physicsSystem.Shutdown();
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;

        KarbonImGUI::shutdown();
        std::cout << "Closing application..." << std::endl;
    }

    void Application::quit() {
        m_running=false;
    }

    bool Application::OnWindowClose(WindowCloseEvent& e) {
        quit();
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e) {
        if(e.getWidth() == 0 || e.getHeight() == 0) {
            m_minimised = true;
            return false;
        }
        m_minimised = false;
        //Renderer on resize

        return false;
    }

    bool Application::OnKeyPress(KeyPressEvent& e) {
        if(e.getKeyCode() == GLFW_KEY_Q) quit();
        if(e.getKeyCode() == GLFW_KEY_ESCAPE) {
            //Toggle mouse visible
            GLFWwindow* window = m_window->getGLWindow();
            int mode = glfwGetInputMode(window, GLFW_CURSOR);
            if (mode == GLFW_CURSOR_NORMAL) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
        return true;
    }

    void Application::OnEvent(Event& e) {
        InputSystem::Get().OnEvent(e);
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(KB_BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(KB_BIND_EVENT_FN(Application::OnWindowResize));
        dispatcher.Dispatch<KeyPressEvent>(KB_BIND_EVENT_FN(Application::OnKeyPress));
    }
    
    float Application::getTime() {
        return static_cast<float>(glfwGetTime());
    }

    float Application::getDeltaTime() {
        return deltaTime;
    }

}