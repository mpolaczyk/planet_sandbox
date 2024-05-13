## PlanetSandbox
**Planet Sandbox**

Author: planet620

This is a fork of the previous project [2.3] (Ray Tracer release 2.3)
All CPU rendering functionality has been removed.
The goal is to achieve the same result but with DX12 and DXR (long way to go).

Features added so far:

- Engine and editor separated to a DLL and EXE
- DX11 support for simple scene rendering, forward renderer, Phong lighting model
- Custom RTTI and memory menegament for resources like scene objects, assets, shaders etc.
- Persistent soft asset pointers
- FBX Import
- Improved UI
  - Material editor
  - Managed objects list
  - Scene editor
  - Object selection and highlight
  - Combo list for managed objects

Example scene: [Cozy Kitchen] imported from Blender.
![Example output](screen.jpg)

### Third party dependencies
DirectX 11

[ocornut/imgui] v1.87

[nlohmann/json] v3.10.5

[PIX for Windows] v1.0.220124001

[STB Image] v2.27

[Tiny obj] v1.0.6

[spdlog] v1.x

[assimp] v5.4.1

[openfbx] 

[//]: # (links)

   [2.3]: <https://bitbucket.org/planet620/raytracer/commits/tag/release_2.3>
   [ocornut/imgui]: <https://github.com/ocornut/imgui>
   [nlohmann/json]: <https://github.com/nlohmann/json>
   [PIX for Windows]: <https://devblogs.microsoft.com/pix/download>
   [STB Image]: <http://nothings.org/stb>
   [Tiny obj]: <https://github.com/tinyobjloader/tinyobjloader>
   [spdlog]: <https://github.com/gabime/spdlog/tree/v1.x>
   [assimp]: <https://github.com/assimp/assimp>
   [openfbx]: <https://github.com/nem0/OpenFBX>
   [Cozy Kitchen]: <https://www.blender.org/download/demo-files>