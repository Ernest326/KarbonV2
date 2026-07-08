#pragma once
#include "assets/asset_handles.h"
#include "graphics/material_system.h"

namespace Karbon {

struct MeshRendererComponent {
    MeshHandle mesh = INVALID_MESH;
    MaterialHandle material = 0;
    bool visible = true;
    bool castShadows = true;
    bool receiveShadows = true;
};

}
