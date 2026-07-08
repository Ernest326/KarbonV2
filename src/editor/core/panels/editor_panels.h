#pragma once

#include <memory>
#include <vector>
#include <entt/entt.hpp>
#include "core/component/component_inspector.h"

namespace Karbon {

class Scene;

class HierarchyPanel {
public:
    void draw(Scene* scene, entt::entity* selectedEntity);
    bool* openFlag() { return &m_open; }
    bool isOpen() const { return m_open; }

private:
    bool m_open = true;
};

class InspectorPanel {
public:
    InspectorPanel();
    void draw(Scene* scene, entt::entity* selectedEntity);
    bool* openFlag() { return &m_open; }
    bool isOpen() const { return m_open; }

private:
    bool m_open = true;
    std::vector<std::unique_ptr<ComponentInspector>> m_componentInspectors;
};

class ContentBrowserPanel {
public:
    void draw(Scene* scene);
    bool* openFlag() { return &m_open; }
    bool isOpen() const { return m_open; }

private:
    bool m_open = true;
};

class StatsPanel {
public:
    void draw(Scene* scene);
    bool* openFlag() { return &m_open; }
    bool isOpen() const { return m_open; }

private:
    bool m_open = true;
};

} // namespace Karbon
