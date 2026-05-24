# TODO list to keep track of things

ENGINE LAYER:

Core:
- Audio system implementation

- Assimp mesh loading system + animation system
- Some sort of material interface + allow assigning materials to material slots in a mesh
- Simple character controller1

Physics:
- Add a addVelocity, addTorque, addImpulse, addForce functions to the RigidbodyComponent, which delegate to the Jolt physics system
- Mesh collision system

Shading/Lighting:
- Skybox (cubemap + hdr skyboxes)
- Add a lighting system which grabs point lights and passes them as needed into shader
- PBR material system
- Shadow maps maybe, or some sort of dynamic shadow system
- Volumetric fog, fog effects
- Bloom effect
- Ambient Occlusion
- Anti-aliasing

Optimizations:
- Backface culling
- Frustum culling
- Use iterators/optimised math libraries

Extra gimmicks:
- OpenVR implementation


-- Stuff to add in the future...
- Navigation system, navmesh baking
- Crossplatform compability




EDITOR:

- ImGUI integration/simple UI
- Some way to compile game engine and allow editor to use the game engine
- Asset system
- Compiler system
- Scene system
- Script compiling system of some sort?