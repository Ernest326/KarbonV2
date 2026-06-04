# TODO list to keep track of things

ENGINE LAYER:

Core:
- Logging system i.e. logger.info(),warn(),debug() + save logs if needed, better observability + same formatting for messages etc. [X]
- ^ Start using the logging system implementation
- Audio system implementation

- Assimp mesh loading system + animation system -> test to see if models load corretly
- Some sort of material interface + allow assigning materials to material slots in a mesh [X]
- Simple character controller

Physics:
- Add a addVelocity, addTorque, addImpulse, addForce functions to the RigidbodyComponent, which delegate to the Jolt physics system
- Add physics event handlers for OnTriggerEnter/OnTriggerExit, OnCollisionEnter/OnCollisionExit, etc.
- Mesh collision system

Rendering:
- Framebuffer system [X]
- Post-process effects templates/settings
- Anti-aliasing

Shading/Lighting:
- More lighting types e.g. Spot, Directional, Area etc.
- Emission separate framebuffer pass HDR/Bloom/Lighting effects?
- Skybox (cubemap + hdr skyboxes) [X]
- Add a lighting system which grabs point lights and passes them as needed into shader [X]
- PBR material system
- Shadow maps maybe, or some sort of dynamic shadow system
- Volumetric fog, fog effects
- Bloom effect
- Ambient Occlusion (SSAO + AO textures)

Optimizations:
- Backface culling
- Frustum culling
- Use iterators/optimised math libraries

Extra gimmicks:
- OpenVR implementation
- Navmesh stuff + basic A* pathfinding
- Vehicle system
- Crossplatform compability
- Networking


EDITOR:

- ImGUI integration/simple UI [X]
- Drag and drop parent system, right click functionality(currently goes into camera look-around no matter what)
- Allow object to be set as active/inactive
- Adding/Deleting Components, Adding material modifiers in meshRenderer material
- Meshrenderer to handle a model with multiple material slots (treat multiple material-separate meshes as a single mesh)
- Some way to compile game engine and allow editor to use the game engine
- Asset system
^ - Scene serialization
  - Materialhandle serialization
  - Serialise a material file which uses texture paths, create/update materialHandler and keep reference of texture paths
- Compiler system
- Scene system [X]
^ - Further develop scene system to save/load a scene, allow play-test and return back to original state once done
- Script compiling system of some sort? (Lua runtime or C/C++)
- Action events + action clipboard for undo/redo
- keybind mapping from a settings file, Keybind -> Action