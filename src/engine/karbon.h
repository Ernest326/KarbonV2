#pragma once

// Karbon engine public API umbrella header.
// Client applications (editor, game, sandbox) should include this.

// Core
#include "core/application.h"
#include "core/window.h"
#include "core/layer.h"
#include "core/layer_stack.h"

// Events & input
#include "events/event.h"
#include "events/application_event.h"
#include "events/key_event.h"
#include "events/mouse_event.h"
#include "events/keycode.h"
#include "events/mousecode.h"
#include "input/inputsystem.h"

// Graphics
#include "graphics/camera.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "graphics/framebuffer.h"
#include "graphics/cubemap.h"
#include "graphics/environment_map.h"
#include "graphics/render_system.h"
#include "graphics/material_system.h"
#include "graphics/lighting_system.h"

// Assets
#include "assets/asset_manager.h"

// Scene & ECS
#include "scene/scene.h"
#include "scene/entity.h"
#include "scene/mesh.h"
#include "scene/model.h"
#include "scene/components/id_component.h"
#include "scene/components/transform.h"
#include "scene/components/hierarchy_component.h"
#include "scene/components/camera_component.h"
#include "scene/components/meshrenderer_component.h"
#include "scene/components/pointlight_component.h"
#include "scene/components/directional_light_component.h"
#include "scene/components/spotlight_component.h"
#include "scene/components/rigidbody_component.h"
#include "scene/components/collider_component.h"

// Physics
#include "physics/physics_system.h"

// Utils
#include "utils/logger.h"
#include "utils/math_utils.h"
