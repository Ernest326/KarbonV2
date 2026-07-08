#pragma once
#include <cstdint>

// Lightweight handle typedefs so components can store asset references
// without pulling in the full AssetManager. Follows the MaterialHandle pattern.

namespace Karbon {

using MeshHandle = uint32_t;
constexpr MeshHandle INVALID_MESH = 0xFFFFFFFFu;

using TextureHandle = uint32_t;
constexpr TextureHandle INVALID_TEXTURE = 0xFFFFFFFFu;

}
