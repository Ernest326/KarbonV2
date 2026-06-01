#pragma once

#include <memory>
#include <vector>
#include <entt/entt.hpp>

namespace Karbon {

class Scene;

class ComponentInspector {
public:
    virtual ~ComponentInspector() = default;
    virtual const char* GetName() const = 0;
    virtual bool CanInspect(entt::registry& registry, entt::entity entity) const = 0;
    virtual void Inspect(entt::registry& registry, entt::entity entity) = 0;
};

class HierarchyPanel {
public:
    void Draw(Scene* scene, entt::entity* selectedEntity);
    bool* OpenFlag() { return &m_open; }
    bool IsOpen() const { return m_open; }

private:
    bool m_open = true;
};

class InspectorPanel {
public:
    InspectorPanel();
    void Draw(Scene* scene, entt::entity* selectedEntity);
    bool* OpenFlag() { return &m_open; }
    bool IsOpen() const { return m_open; }

private:
    bool m_open = true;
    std::vector<std::unique_ptr<ComponentInspector>> m_componentInspectors;
};

class ContentBrowserPanel {
public:
    void Draw(Scene* scene);
    bool* OpenFlag() { return &m_open; }
    bool IsOpen() const { return m_open; }

private:
    bool m_open = true;
};

class StatsPanel {
public:
    void Draw(Scene* scene);
    bool* OpenFlag() { return &m_open; }
    bool IsOpen() const { return m_open; }

private:
    bool m_open = true;
};

} // namespace Karbon
