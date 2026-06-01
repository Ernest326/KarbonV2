# TODO list to keep track of things

ENGINE LAYER:

Core:
- Logging system i.e. logger.info(),warn(),debug() + save logs if needed, better observability + same formatting for messages etc.
- Audio system implementation

- Assimp mesh loading system + animation system -> test to see if models load corretly
- Some sort of material interface + allow assigning materials to material slots in a mesh [X]
- Simple character controller

Physics:
- Add a addVelocity, addTorque, addImpulse, addForce functions to the RigidbodyComponent, which delegate to the Jolt physics system
- Add physics event handlers for OnTriggerEnter/OnTriggerExit, OnCollisionEnter/OnCollisionExit, etc.
- Mesh collision system

Rendering:
- Framebuffer system
- Anti-aliasing

Shading/Lighting:
- Skybox (cubemap + hdr skyboxes) [X]
- Add a lighting system which grabs point lights and passes them as needed into shader [X]
- PBR material system
- Shadow maps maybe, or some sort of dynamic shadow system
- Volumetric fog, fog effects
- Bloom effect
- Ambient Occlusion

Optimizations:
- Backface culling
- Frustum culling
- Use iterators/optimised math libraries

Extra gimmicks:
- OpenVR implementation


-- Stuff to add in the future...
- Navigation system, navmesh baking
- Crossplatform compability
- Networking



EDITOR:

- ImGUI integration/simple UI [X]
- Some way to compile game engine and allow editor to use the game engine
- Asset system
- Compiler system
- Scene system [X]
- Script compiling system of some sort?