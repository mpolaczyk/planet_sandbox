## Planet sandbox
**GPU Rendering sandbox**

Author: planet620

This is a fork of the previous project [2.3] (Ray Tracer release 2.3)
All CPU rendering functionality has been removed.
The goal is to achieve the same result but with DX12 and DXR.

Features added so far:

2.0 - Work in progress
- DX12 support

[1.0] - Initial release
- DX11 support for simple scene rendering, forward renderer, deferred renderer, Phong lighting model
- Engine and editor separated to a DLL and EXE
- Custom RTTI and memory menegament for resources like scene objects, assets, shaders etc.
- Persistent soft asset pointers
- FBX scene import
- Support to multiple project folders (use command line argument fbx_scene_test or cozy_kitchen)
- Improved UI
    - Material editor
    - Managed objects list
    - Scene editor
    - Object selection and highlight
    - Combo list for managed objects
    - Object selection

Example scene: [Cozy Kitchen] imported from Blender.
![Example output](screen.jpg)

### Third party dependencies
DirectX 12

[Nsight Aftermath SDK] v2024.2.0.24200

[DirectX 12 Agility SDK] v1.614

[DirectX Shader Compiler] v1.8.2407

[ocornut/imgui] v1.87

[nlohmann/json] v3.10.5

[PIX for Windows] v1.0.220124001

[STB Image] v2.27

[Tiny obj] v1.0.6

[spdlog] v1.x

[assimp] v5.4.1

[openfbx] 

[//]: # (links)

   [1.0]: <https://bitbucket.org/planet620/planetsandbox/src/release_1.0/>
   [2.3]: <https://bitbucket.org/planet620/raytracer/src/release_2.3/>
   [ocornut/imgui]: <https://github.com/ocornut/imgui>
   [nlohmann/json]: <https://github.com/nlohmann/json>
   [PIX for Windows]: <https://devblogs.microsoft.com/pix/download>
   [STB Image]: <http://nothings.org/stb>
   [Tiny obj]: <https://github.com/tinyobjloader/tinyobjloader>
   [spdlog]: <https://github.com/gabime/spdlog/tree/v1.x>
   [assimp]: <https://github.com/assimp/assimp>
   [openfbx]: <https://github.com/nem0/OpenFBX>
   [Cozy Kitchen]: <https://www.blender.org/download/demo-files>
   [DirectX 12 Agility SDK]: <https://www.nuget.org/packages/Microsoft.Direct3D.D3D12/1.614.0>
   [DirectX Shader Compiler]: <https://github.com/microsoft/DirectXShaderCompiler/releases/tag/v1.8.2407>
   [Nsight Aftermath SDK]: <https://github.com/NVIDIA/nsight-aftermath-samples/tree/master>