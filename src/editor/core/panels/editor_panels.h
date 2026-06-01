#pragma once

#include <entt/entt.hpp>

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
    void Draw(Scene* scene);
    bool* OpenFlag() { return &m_open; }
    bool IsOpen() const { return m_open; }

private:
    bool m_open = true;
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
