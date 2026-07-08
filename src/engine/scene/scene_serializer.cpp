#include "scene/scene_serializer.h"
#include "scene/scene.h"
#include "assets/asset_manager.h"
#include "scene/components/id_component.h"
#include "scene/components/hierarchy_component.h"
#include "scene/components/transform.h"
#include "scene/components/camera_component.h"
#include "scene/components/meshrenderer_component.h"
#include "scene/components/pointlight_component.h"
#include "scene/components/directional_light_component.h"
#include "scene/components/spotlight_component.h"
#include "scene/components/rigidbody_component.h"
#include "scene/components/collider_component.h"
#include "utils/logger.h"
#include <glm/gtc/quaternion.hpp>
#include <fstream>
#include <sstream>
#include <iterator>
#include <unordered_map>
#include <vector>

namespace Karbon {

namespace {

// ---- tiny section-based text format ("[Name]" header, "key = value" fields) ----

struct Section {
    std::string name;
    std::unordered_map<std::string, std::string> fields;

    std::string get(const std::string& key, const std::string& fallback = "") const {
        auto it = fields.find(key);
        return it != fields.end() ? it->second : fallback;
    }
};

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::vector<std::string> splitWhitespace(const std::string& s) {
    std::istringstream iss(s);
    return {std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>()};
}

float parseFloatField(const std::string& value, float fallback) {
    try { return std::stof(value); } catch (const std::exception&) { return fallback; }
}

int parseIntField(const std::string& value, int fallback) {
    try { return std::stoi(value); } catch (const std::exception&) { return fallback; }
}

UUID parseUUIDField(const std::string& value, UUID fallback = 0) {
    try { return static_cast<UUID>(std::stoull(value)); } catch (const std::exception&) { return fallback; }
}

bool parseBoolField(const std::string& value, bool fallback) {
    if (value == "1" || value == "true") return true;
    if (value == "0" || value == "false") return false;
    return fallback;
}

// Leaves components at their fallback value if the stream runs out early
glm::vec3 parseVec3(const std::string& value, const glm::vec3& fallback) {
    std::istringstream iss(value);
    glm::vec3 result = fallback;
    iss >> result.x >> result.y >> result.z;
    return result;
}

glm::quat parseQuat(const std::string& value, const glm::quat& fallback) {
    std::istringstream iss(value);
    float w = fallback.w, x = fallback.x, y = fallback.y, z = fallback.z;
    iss >> w >> x >> y >> z;
    return glm::quat(w, x, y, z);
}

std::vector<Section> parseSections(const std::string& path) {
    std::vector<Section> sections;
    std::ifstream file(path);
    if (!file.is_open()) return sections;

    std::string line;
    while (std::getline(file, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#') continue;

        if (trimmed.front() == '[' && trimmed.back() == ']') {
            sections.push_back({trimmed.substr(1, trimmed.size() - 2), {}});
            continue;
        }

        if (sections.empty()) continue; // stray field before any section header

        size_t eq = trimmed.find('=');
        if (eq == std::string::npos) continue;
        std::string key = trim(trimmed.substr(0, eq));
        std::string value = trim(trimmed.substr(eq + 1));
        sections.back().fields[key] = value;
    }
    return sections;
}

// ---- writer helpers ----

void writeSection(std::ofstream& out, const char* name) { out << "[" << name << "]\n"; }
void writeField(std::ofstream& out, const char* key, const std::string& value) { out << key << " = " << value << "\n"; }
void writeField(std::ofstream& out, const char* key, float value) { out << key << " = " << value << "\n"; }
void writeField(std::ofstream& out, const char* key, bool value) { out << key << " = " << (value ? 1 : 0) << "\n"; }
void writeField(std::ofstream& out, const char* key, uint32_t value) { out << key << " = " << value << "\n"; }
void writeField(std::ofstream& out, const char* key, const glm::vec3& v) { out << key << " = " << v.x << " " << v.y << " " << v.z << "\n"; }
void writeField(std::ofstream& out, const char* key, const glm::quat& q) { out << key << " = " << q.w << " " << q.x << " " << q.y << " " << q.z << "\n"; }
void writeUUIDField(std::ofstream& out, const char* key, UUID id) { out << key << " = " << id << "\n"; }

MeshHandle resolvePrimitiveMesh(AssetManager& assets, const std::string& name) {
    if (name == "cube") return assets.getCubeMesh();
    if (name == "sphere") return assets.getSphereMesh();
    if (name == "plane") return assets.getPlaneMesh();
    return INVALID_MESH;
}

} // namespace

bool saveScene(Scene& scene, AssetManager& assets, const std::string& path) {
    std::ofstream out(path);
    if (!out.is_open()) {
        Logger::get().error("SceneSerializer: failed to write '" + path + "'");
        return false;
    }

    auto& registry = scene.getRegistry();

    // Scene-level metadata
    writeSection(out, "Scene");
    UUID primaryCameraId = 0;
    entt::entity primaryCameraEntity = scene.getPrimaryCameraEntity();
    if (registry.valid(primaryCameraEntity) && registry.all_of<IDComponent, TagComponent>(primaryCameraEntity)
        && registry.get<TagComponent>(primaryCameraEntity).tag != "Editor Camera") {
        primaryCameraId = registry.get<IDComponent>(primaryCameraEntity).id;
    }
    writeUUIDField(out, "primaryCamera", primaryCameraId);
    out << "\n";

    // Environment
    const SceneEnvironment& env = scene.getEnvironment();
    writeSection(out, "Environment");
    switch (env.getType()) {
        case SceneEnvironment::Type::HDR:
            writeField(out, "type", std::string("hdr"));
            writeField(out, "hdrPath", env.getHDRPath());
            break;
        case SceneEnvironment::Type::Cubemap: {
            writeField(out, "type", std::string("cubemap"));
            std::ostringstream faces;
            for (const auto& f : env.getCubemapFaces()) faces << f << " ";
            writeField(out, "cubemapFaces", faces.str());
            break;
        }
        default:
            writeField(out, "type", std::string("none"));
            break;
    }
    out << "\n";

    // Entities: every entity created via Scene::createEntity(WithID) has these 4.
    // The editor's own utility camera ("Editor Camera") is excluded — it's
    // recreated fresh by EditorCamera::initialize() every session, not scene content.
    auto view = registry.view<IDComponent, TagComponent, HierarchyComponent, TransformComponent>();
    for (auto entity : view) {
        auto& id = view.get<IDComponent>(entity);
        auto& tag = view.get<TagComponent>(entity);
        auto& hierarchy = view.get<HierarchyComponent>(entity);
        auto& transform = view.get<TransformComponent>(entity);

        if (tag.tag == "Editor Camera") continue;

        UUID parentId = 0;
        if (hierarchy.parent != entt::null && registry.valid(hierarchy.parent) && registry.all_of<IDComponent>(hierarchy.parent)) {
            parentId = registry.get<IDComponent>(hierarchy.parent).id;
        }

        writeSection(out, "Entity");
        writeUUIDField(out, "uuid", id.id);
        writeField(out, "tag", tag.tag);
        writeUUIDField(out, "parent", parentId);
        out << "\n";

        writeSection(out, "Transform");
        writeField(out, "position", transform.position);
        writeField(out, "rotation", transform.rotation);
        writeField(out, "scale", transform.scale);
        out << "\n";

        if (auto* renderer = registry.try_get<MeshRendererComponent>(entity)) {
            if (const MeshSource* source = assets.getMeshSource(renderer->mesh)) {
                writeSection(out, "MeshRenderer");
                writeField(out, "meshType", source->type == MeshSource::Type::Primitive ? std::string("primitive") : std::string("model"));
                writeField(out, "meshPath", source->path);
                writeField(out, "subMesh", source->subMeshIndex);

                // Materials created directly through MaterialSystem (no backing file)
                // get one generated here so the reference survives the round trip.
                std::string materialPath = assets.getMaterialPath(renderer->material);
                if (materialPath.empty()) {
                    materialPath = "resources/materials/mat_" + std::to_string(renderer->material) + ".kmat";
                    assets.saveMaterial(materialPath, renderer->material);
                }
                writeField(out, "material", materialPath);

                writeField(out, "visible", renderer->visible);
                writeField(out, "castShadows", renderer->castShadows);
                writeField(out, "receiveShadows", renderer->receiveShadows);
                out << "\n";
            }
        }

        if (auto* light = registry.try_get<PointLightComponent>(entity)) {
            writeSection(out, "PointLight");
            writeField(out, "color", light->color);
            writeField(out, "intensity", light->intensity);
            writeField(out, "radius", light->radius);
            writeField(out, "falloff", light->falloff);
            writeField(out, "castShadows", light->castShadows);
            out << "\n";
        }

        if (auto* light = registry.try_get<DirectionalLightComponent>(entity)) {
            writeSection(out, "DirectionalLight");
            writeField(out, "color", light->color);
            writeField(out, "intensity", light->intensity);
            writeField(out, "castShadows", light->castShadows);
            out << "\n";
        }

        if (auto* light = registry.try_get<SpotLightComponent>(entity)) {
            writeSection(out, "SpotLight");
            writeField(out, "color", light->color);
            writeField(out, "intensity", light->intensity);
            writeField(out, "radius", light->radius);
            writeField(out, "falloff", light->falloff);
            writeField(out, "cutOff", light->cutOff);
            writeField(out, "outerCutOff", light->outerCutOff);
            writeField(out, "castShadows", light->castShadows);
            out << "\n";
        }

        if (auto* cam = registry.try_get<CameraComponent>(entity)) {
            writeSection(out, "Camera");
            writeField(out, "fov", cam->camera.getFov());
            writeField(out, "nearPlane", cam->camera.getNearPlane());
            writeField(out, "farPlane", cam->camera.getFarPlane());
            // The Camera's own position/rotation are authoritative for camera
            // entities (controllers mutate the Camera directly, not Transform).
            writeField(out, "position", cam->camera.getPosition());
            writeField(out, "rotation", cam->camera.getRotation());
            out << "\n";
        }

        if (auto* rb = registry.try_get<RigidbodyComponent>(entity)) {
            writeSection(out, "Rigidbody");
            const char* typeStr = rb->type == RigidbodyComponent::Type::Static ? "static"
                                 : rb->type == RigidbodyComponent::Type::Kinematic ? "kinematic" : "dynamic";
            writeField(out, "type", std::string(typeStr));
            writeField(out, "physicsType", rb->physicsType == RigidbodyComponent::PhysicsType::Continuous ? std::string("continuous") : std::string("discrete"));
            writeField(out, "mass", rb->mass);
            writeField(out, "friction", rb->friction);
            writeField(out, "restitution", rb->restitution);
            writeField(out, "isTrigger", rb->isTrigger);
            out << "\n";
        }

        if (auto* col = registry.try_get<ColliderComponent>(entity)) {
            writeSection(out, "Collider");
            const char* typeStr = col->type == ColliderComponent::Type::Box ? "box"
                                 : col->type == ColliderComponent::Type::Sphere ? "sphere"
                                 : col->type == ColliderComponent::Type::Capsule ? "capsule"
                                 : col->type == ColliderComponent::Type::Mesh ? "mesh" : "plane";
            writeField(out, "type", std::string(typeStr));
            writeField(out, "halfExtents", col->halfExtents);
            writeField(out, "radius", col->radius);
            writeField(out, "offset", col->offset);
            out << "\n";
        }
    }

    Logger::get().info("SceneSerializer: saved scene to '" + path + "'");
    return true;
}

bool loadScene(Scene& scene, AssetManager& assets, const std::string& path) {
    std::vector<Section> sections = parseSections(path);
    if (sections.empty()) {
        Logger::get().error("SceneSerializer: failed to open or empty scene file '" + path + "'");
        return false;
    }

    scene.clear();

    UUID primaryCameraId = 0;
    entt::entity currentEntity = entt::null;
    std::vector<std::pair<entt::entity, UUID>> pendingParents;

    for (const Section& section : sections) {
        if (section.name == "Scene") {
            primaryCameraId = parseUUIDField(section.get("primaryCamera"), 0);

        } else if (section.name == "Environment") {
            std::string type = section.get("type", "none");
            if (type == "hdr") {
                scene.getEnvironment().setHDR(section.get("hdrPath"));
                scene.getEnvironment().generateIBL();
            } else if (type == "cubemap") {
                auto faces = splitWhitespace(section.get("cubemapFaces"));
                if (faces.size() == 6) {
                    scene.getEnvironment().setCubemap(faces);
                    scene.getEnvironment().generateIBL();
                }
            }

        } else if (section.name == "Entity") {
            UUID id = parseUUIDField(section.get("uuid"), 0);
            std::string tag = section.get("tag", "Entity");
            UUID parentId = parseUUIDField(section.get("parent"), 0);

            currentEntity = scene.createEntityWithID(id, tag);
            if (parentId != 0) {
                pendingParents.push_back({currentEntity, parentId});
            }

        } else if (section.name == "Transform") {
            if (currentEntity == entt::null) continue;
            auto& transform = scene.getRegistry().get<TransformComponent>(currentEntity);
            transform.position = parseVec3(section.get("position"), transform.position);
            transform.rotation = parseQuat(section.get("rotation"), transform.rotation);
            transform.scale = parseVec3(section.get("scale"), transform.scale);

        } else if (section.name == "MeshRenderer") {
            if (currentEntity == entt::null) continue;
            std::string meshType = section.get("meshType");
            std::string meshPath = section.get("meshPath");
            uint32_t subMesh = static_cast<uint32_t>(parseIntField(section.get("subMesh"), 0));

            MeshHandle mesh = INVALID_MESH;
            if (meshType == "primitive") {
                mesh = resolvePrimitiveMesh(assets, meshPath);
            } else if (meshType == "model") {
                auto meshes = assets.loadModel(meshPath);
                if (subMesh < meshes.size()) mesh = meshes[subMesh];
            }

            std::string materialPath = section.get("material");
            MaterialHandle material = materialPath.empty() ? 0 : assets.loadMaterial(materialPath);

            auto& renderer = scene.getRegistry().emplace_or_replace<MeshRendererComponent>(currentEntity);
            renderer.mesh = mesh;
            renderer.material = material;
            renderer.visible = parseBoolField(section.get("visible"), true);
            renderer.castShadows = parseBoolField(section.get("castShadows"), true);
            renderer.receiveShadows = parseBoolField(section.get("receiveShadows"), true);

        } else if (section.name == "PointLight") {
            if (currentEntity == entt::null) continue;
            auto& light = scene.getRegistry().emplace_or_replace<PointLightComponent>(currentEntity);
            light.color = parseVec3(section.get("color"), light.color);
            light.intensity = parseFloatField(section.get("intensity"), light.intensity);
            light.radius = parseFloatField(section.get("radius"), light.radius);
            light.falloff = parseFloatField(section.get("falloff"), light.falloff);
            light.castShadows = parseBoolField(section.get("castShadows"), light.castShadows);

        } else if (section.name == "DirectionalLight") {
            if (currentEntity == entt::null) continue;
            auto& light = scene.getRegistry().emplace_or_replace<DirectionalLightComponent>(currentEntity);
            light.color = parseVec3(section.get("color"), light.color);
            light.intensity = parseFloatField(section.get("intensity"), light.intensity);
            light.castShadows = parseBoolField(section.get("castShadows"), light.castShadows);

        } else if (section.name == "SpotLight") {
            if (currentEntity == entt::null) continue;
            auto& light = scene.getRegistry().emplace_or_replace<SpotLightComponent>(currentEntity);
            light.color = parseVec3(section.get("color"), light.color);
            light.intensity = parseFloatField(section.get("intensity"), light.intensity);
            light.radius = parseFloatField(section.get("radius"), light.radius);
            light.falloff = parseFloatField(section.get("falloff"), light.falloff);
            light.cutOff = parseFloatField(section.get("cutOff"), light.cutOff);
            light.outerCutOff = parseFloatField(section.get("outerCutOff"), light.outerCutOff);
            light.castShadows = parseBoolField(section.get("castShadows"), light.castShadows);

        } else if (section.name == "Camera") {
            if (currentEntity == entt::null) continue;
            auto& cam = scene.getRegistry().emplace_or_replace<CameraComponent>(currentEntity);
            float fov = parseFloatField(section.get("fov"), cam.camera.getFov());
            float nearPlane = parseFloatField(section.get("nearPlane"), cam.camera.getNearPlane());
            float farPlane = parseFloatField(section.get("farPlane"), cam.camera.getFarPlane());
            cam.camera.setProjection(fov, nearPlane, farPlane);
            cam.camera.setPosition(parseVec3(section.get("position"), cam.camera.getPosition()));
            cam.camera.setRotation(parseVec3(section.get("rotation"), cam.camera.getRotation()));

        } else if (section.name == "Rigidbody") {
            if (currentEntity == entt::null) continue;
            auto& rb = scene.getRegistry().emplace_or_replace<RigidbodyComponent>(currentEntity);
            std::string typeStr = section.get("type", "static");
            rb.type = typeStr == "dynamic" ? RigidbodyComponent::Type::Dynamic
                     : typeStr == "kinematic" ? RigidbodyComponent::Type::Kinematic
                     : RigidbodyComponent::Type::Static;
            rb.physicsType = section.get("physicsType") == "continuous" ? RigidbodyComponent::PhysicsType::Continuous : RigidbodyComponent::PhysicsType::Discrete;
            rb.mass = parseFloatField(section.get("mass"), rb.mass);
            rb.friction = parseFloatField(section.get("friction"), rb.friction);
            rb.restitution = parseFloatField(section.get("restitution"), rb.restitution);
            rb.isTrigger = parseBoolField(section.get("isTrigger"), rb.isTrigger);
            rb.initialized = false;

        } else if (section.name == "Collider") {
            if (currentEntity == entt::null) continue;
            auto& col = scene.getRegistry().emplace_or_replace<ColliderComponent>(currentEntity);
            std::string typeStr = section.get("type", "box");
            col.type = typeStr == "sphere" ? ColliderComponent::Type::Sphere
                     : typeStr == "capsule" ? ColliderComponent::Type::Capsule
                     : typeStr == "mesh" ? ColliderComponent::Type::Mesh
                     : typeStr == "plane" ? ColliderComponent::Type::Plane
                     : ColliderComponent::Type::Box;
            col.halfExtents = parseVec3(section.get("halfExtents"), col.halfExtents);
            col.radius = parseFloatField(section.get("radius"), col.radius);
            col.offset = parseVec3(section.get("offset"), col.offset);
        }
    }

    // Second pass: wire up parenting now that every entity exists
    for (auto& [child, parentId] : pendingParents) {
        entt::entity parent = scene.findByUUID(parentId);
        if (parent != entt::null) {
            scene.setParent(child, parent);
        }
    }

    if (primaryCameraId != 0) {
        entt::entity camEntity = scene.findByUUID(primaryCameraId);
        if (camEntity != entt::null) {
            scene.setPrimaryCamera(camEntity);
        }
    }

    scene.onUpdate();

    Logger::get().info("SceneSerializer: loaded scene from '" + path + "'");
    return true;
}

}
