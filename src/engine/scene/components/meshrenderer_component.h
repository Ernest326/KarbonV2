#pragma once
#include "../../graphics/material_system.h"
#include "../mesh.h"

namespace Karbon {

struct MeshRendererComponent {
    Mesh* mesh = nullptr;
    MaterialHandle material = 0;
    bool visible = true;
    bool castShadows = true;
    bool receiveShadows = true;
};

}