# Ark-Engine
<img width="1169" height="570" alt="image" src="https://github.com/user-attachments/assets/68ac19ea-107f-443a-9952-214eafed9048" />

<img width="2552" height="1527" alt="image" src="https://github.com/user-attachments/assets/b59126c7-eee8-4262-98ee-258b0720ecb3" />

# 🛠️ Ark-Engine

**Ark-Engine** is a modular C++ game engine built with modern standards and a focus on performance, flexibility, and extensibility. It leverages **C++20** and is compiled using **MSVC**, with rendering powered by **Vulkan 1.4.328.1**.

The Main Files starts by Enabling the logger, then it checks for which graphics API that you wanna use, you can simply edit it in the main if you wanna utilize OpenGL or Vulkan. (Note: Still in development) 
It's a pisser to plan and structure this mess but surely it will workout fine. 
---
+------------------------------------------------------+
|                    Application Layer                 |
|  (main.cpp, Editor UI, Scene Controls, Input)        |
+---------------------------+--------------------------+
|         Engine Core       |      Rendering Engine    |
|  (Scene, ECS, Assets)     |  (OpenGL, Shaders, Grid) |
+---------------------------+--------------------------+
|                Platform Layer (GLFW, GLAD)           |
+------------------------------------------------------+

## 🔧 Core Technologies
- **Language:** C++20  
- **Graphics API:** Vulkan 1.4 & OpenGL 3.4
- **Build System:** Just a Git Clone. CMake did't feel practical, i am more used to installing a thing and having it be done.  
- **Compiler:** MSVC  

** KeyBinds
G - Toggles the grid
H - Toggles wireframe mode

Navigation movement with ASWD
Rotational movement with Mouse X and Y
Ascend Descend with Q and E.
---

## 📦 Dependencies
- [GLM](https://github.com/g-truc/glm) — mathematics for graphics  
- [GLFW](https://github.com/glfw/glfw) — window and input handling  
- [Assimp](https://github.com/assimp/assimp) — asset import library  
- [stb_image](https://github.com/nothings/stb) — image loading  
- [Compressonator](https://github.com/GPUOpen-Tools/compressonator) - Compressor tool
- [FMod](https://fmod.com) - Audio Manager / Audio Player
- [ImGui](https://github.com/ocornut/imgui) - Edito[rUI Library
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) - Gizmo model
- [MeshOptimizer](https://github.com/zeux/meshoptimizer) - MeshOptimizer
- [PhysX](https://github.com/NVIDIA-Omniverse/PhysX) - PHysics system (Not utilized yet)
- [SDL3](https://github.com/libsdl-org/SDL) - Optional Window creator/handler
- [Vulkan](https://www.vulkan.org/) - Vulkan Graphical Library (Not in use yet - not enough time to implement)
- [ZLib](https://github.com/madler/zlib) - Zip library
- [MagicEnum](https://github.com/Neargye/magic_enum) - Magic Enum Utility
- [FreeType2](https://github.com/aseprite/freetype2) - FontHandler
- [ENTT](https://github.com/skypjack/entt) - Entity Component Registy System
- [TinyExr](https://github.com/syoyo/tinyexr) - support for .exr files
- [Nlohmann](https://github.com/nlohmann/json) - Json parser 
---
## Engine Resources and Research Notes

Sky Rendering: https://www.youtube.com/watch?v=zZzLlRyOnZ8

API Entry point and utility libraries: HellEngine https://www.youtube.com/watch?v=4s2M-e39nBM

Overall good resource repository to learn from: https://github.com/emeiri/ogldev

File Compression Optimization: https://www.youtube.com/watch?v=-9VlEtCUN0I

Assimp: https://www.youtube.com/watch?v=r4MfFTDj0VU&t=30s

## 🚧 Planned Features
- **Editor** — in-engine editing and visualization tools  
- **Texture System** — loading, management, and runtime usage  
- **Shader Pipeline** — modular shader support with Vulkan integration  
- **Dynamic Rendering** — real-time rendering with GPU-driven pipelines  
- **Multithreading** — task-based parallelism for performance  
- **UI Framework** — custom or integrated GUI system  
- **ECS Architecture** — entity-component-system for scalable game logic  
- **Package Manager** — modular asset and code distribution  
- **Multithreaded Command Buff**

- Potentially might add a tarpit file to prevent AI llms learning and reading up on my project model file. - https://www.youtube.com/watch?v=vC2mlCtuJiU
---

## 📖 Getting Started
Open up your command prompt or powershell and git clone it. All dependancies are pathed properly and should work out alright. 
Be sure to run the .exe in the Export folder instead of compiling it inside of visual studio to get more accurate results.
