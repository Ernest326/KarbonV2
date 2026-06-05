#pragma once
#include <entt/entt.hpp>
#include <unordered_map>
#include <string>
#include "components/camera_component.h"
#include <memory>
#include <glm/glm.hpp>
#include "components/id_component.h"
#include "components/transform.h"
#include "../graphics/cubemap.h"
#include "../graphics/environment_map.h"

namespace Karbon {

class SceneEnvironment {
public:
    enum class Type { None, Cubemap, HDR };

    void setCubemap(const std::vector<std::string>& faces);
    void setHDR(const std::string& hdrPath);
    void clear();

    Type getType() const { return m_type; }

    unsigned int getSkyboxCubemap() const;

    bool hasIBL() const;
    void generateIBL();
    unsigned int getIrradianceMap() const;
    unsigned int getPrefilterMap() const;
    unsigned int getBRDFLUT() const;
    void bind();
    void unbind();
    void bindIBL(Shader& shader);

    const std::vector<std::string>& getCubemapFaces() const { return m_cubemapFaces; }
    const std::string& getHDRPath() const { return m_hdrPath; }

private:
    Type m_type = Type::None;
    std::unique_ptr<Cubemap> m_cubemap;
    std::unique_ptr<EnvironmentMap> m_environmentMap;

    std::vector<std::string> m_cubemapFaces;
    std::string m_hdrPath;
};

class Scene {
public:
    Scene() = default;
    ~Scene() = default;

    entt::entity createEntity(const std::string& tag="Entity");
    void destroyEntity(entt::entity entity);
    entt::entity findByTag(const std::string& tag);
    entt::entity findByUUID(UUID id);

    void unparent(entt::entity child);
    void setParent(entt::entity child, entt::entity parent);

    void setPrimaryCamera(entt::entity entity);
    Camera* getPrimaryCamera();
    entt::entity getPrimaryCameraEntity() const { return m_primaryCamera; }
    entt::registry& getRegistry() { return m_registry; }

    void onUpdate();

    void markDirtyUpward(entt::entity entity);
    void markDirtyDownward(entt::entity entity);

    //Environment/Skybox
    SceneEnvironment& getEnvironment() { return m_sceneEnvironment; }
    const SceneEnvironment& getEnvironment() const { return m_sceneEnvironment; } 

    void renderSkybox(Shader& skyboxShader);

private:
    entt::registry m_registry;
    std::unordered_map<UUID, entt::entity> m_entityMap;
    std::unordered_map<std::string, entt::entity> m_tagMap;
    UUID m_nextUUID = 1;
    entt::entity m_primaryCamera = entt::null;

    SceneEnvironment m_sceneEnvironment;

    void updateHierarchy();
    void updateWorldTransform(entt::entity entity, const WorldTransformComponent& parentWorld);

    void serialize(const std::string& filepath); //TODO for future
    void deserialize(const std::string& filepath); //TODO for future

};

}