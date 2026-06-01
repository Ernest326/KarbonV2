#pragma once

#include <memory>
#include <vector>
#include <entt/entt.hpp>
#include "../component/component_inspector.h"

namespace Karbon {

class Scene;

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
