#pragma once
#include <entt/entt.hpp>
#include <vector>

namespace Karbon {
    struct HierarchyComponent {
        entt::entity parent = entt::null;
        std::vector<entt::entity> children;
        bool dirty = true;
    };
}