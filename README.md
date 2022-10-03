# Horizon Engine

Horizon Engine is an open-source 3D rendering engine, focusing on modern rendering engine architecture and rendering techniques. Serving a different purpose than game engines, this project aims to build a highly scalable rendering framework to improve the productivity of prototype projects and academic research, but also to serve as an educational tool for teaching rendering engine design and implementation from scratch.

Horizon Engine is currently only supported on Windows and only target modern graphics APIs (Direct3D 12, Vulkan, Metal).

<!--
[![Bilibili]()]()
-->
[![Discord](https://badgen.net/badge/icon/discord?icon=discord&label)](https://discord.gg/nepzQHf2jv)
<!--
# [![Patreon](https://badgen.net/badge/icon/patreon?icon=patreon&label)]()
-->

## Introduction

Goals:
* Efficent and Flexible Rendering
* Fast Rendering Techniques Experimentation, eg. Hybrid Rendering with DXR or Vulkan Raytracing

Rendering Architecture:

Inspired by [Halcyon](https://www.ea.com/seed/news/khronos-munich-2018-halcyon-vulkan)

![image](/Docs/rendering_architecture.png)

Implemented features:
* Modularization using C++20 modules
* Fiber-Based Job System
* Bindless
* Render Graph
* ECS (Entity-Component-System)
* Hybrid Render Pipeline
* Deferred Render Pipeline
* Path Tracing

Main features to come:
* Visibility Buffer (Also Called Deferred Texturing or Deferred Material Shading)
* High-Fidelity Physically-Based Rendering
* Surfle GI
* Native Graphics API Support (Direct3D 12, Metal 2)
* Multi-GPU Rendering
* Plugin System (Live-Reloadable Plugins)
* Physics Engine Integration (PhysX)
* Audio System
* Machine Learning
* VR/AR Support
* Cross-Platform (MacOS and Linux Support)

## Samples

Scene Editor
![image](/Screenshots/screenshot_editor.png)

FSR
![image](/Screenshots/screenshot_fsr.png)

Sky Atomsphere
![image](/Screenshots/screenshot_sky_atmosphere.png)

Particles
![image](/Screenshots/screenshot_particle_system.png)

Cornell Box
![image](/Screenshots/screenshot_cornell_box.png)

## Requirements

* Windows 10 SDK (10.0.20348.0)
* NIVIDIA Graphics Cards (Geforce RTX 20-series or 30-series) and keep your graphics drivers up to date (https://www.nvidia.com/Download/index.aspx)
* Vulkan SDK 1.3.216.0, this repository tries to always be up to date with the latest Vulkan SDK (https://vulkan.lunarg.com/sdk/home)
* Visual Studio 2022
* C++ 20

## Getting Started

1. Clone this repository

`git clone https://github.com/harukumo/HorizonEngine`

2. Run "GenerateProjects.bat"

## Third Party

* [assimp](https://github.com/assimp/assimp)
* [imgui](https://github.com/ocornut/imgui)
* [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)
* [entt](https://github.com/skypjack/entt)
* [glfw](https://github.com/glfw/glfw)
* [stb](https://github.com/nothings/stb)
* [spdlog](https://github.com/gabime/spdlog)
* [glm](https://github.com/g-truc/glm)
* [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
* [Vulkan](https://www.khronos.org/vulkan)
* [DirectX-Headers](https://github.com/microsoft/DirectX-Headers)
* [D3D12MemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator)
* [DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler)
* [OpenEXR](https://github.com/AcademySoftwareFoundation/openexr)
* [yaml-cpp](https://github.com/jbeder/yaml-cpp)
* [LLVM](https://github.com/llvm/llvm-project)
* [MPMCQueue](https://github.com/rigtorp/MPMCQueue)
* [PhysX](https://github.com/NVIDIAGameWorks/PhysX)
* [OptiX](https://developer.nvidia.com/rtx/ray-tracing/optix)
* [CUDA](https://developer.nvidia.com/cuda-toolkit)
* [premake](https://github.com/premake/premake-core)

## Documentation


## Debug Tools

[Nsight Graphics](https://developer.nvidia.com/nsight-graphics)

[RenderDoc](https://renderdoc.org/)

## Contribution

Contributions are welcome.

## Sponsors


## Lisence