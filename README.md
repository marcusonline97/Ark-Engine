# Ark-Engine
<img width="1169" height="570" alt="image" src="https://github.com/user-attachments/assets/68ac19ea-107f-443a-9952-214eafed9048" />

<img width="2552" height="1527" alt="image" src="https://github.com/user-attachments/assets/b59126c7-eee8-4262-98ee-258b0720ecb3" />

# 🛠️ Ark-Engine

**Ark-Engine** is a modular C++ game engine built with modern standards and a focus on performance, flexibility, and extensibility. It leverages **C++20** and is compiled using **MSVC**, with rendering powered by **Vulkan 1.4.328.1**.

The Main Files starts by Enabling the logger, then it checks for which graphics API that you wanna use, you can simply edit it in the main if you wanna utilize OpenGL or Vulkan. (Note: Still in development) 
It's a pisser to plan and structure this mess but surely it will workout fine. 
---

## 🔧 Core Technologies
- **Language:** C++20  
- **Graphics API:** Vulkan 1.4 & OpenGL 3.4
- **Build System:** Just a Git Clone. CMake did't feel practical, i am more used to installing a thing and having it be done.  
- **Compiler:** MSVC  

---

## 📦 Dependencies
- [GLM](https://github.com/g-truc/glm) — mathematics for graphics  
- [GLFW](https://github.com/glfw/glfw) — window and input handling  
- [Assimp](https://github.com/assimp/assimp) — asset import library  
- [stb_image](https://github.com/nothings/stb) — image loading  

---
## Engine Resources and Research Notes

Sky Rendering: https://www.youtube.com/watch?v=zZzLlRyOnZ8
Structure: HellEngine https://www.youtube.com/watch?v=4s2M-e39nBM
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

- 
---

## 📖 Getting Started
> CMake integration is planned. Once available, you’ll be able to fetch and build Ark-Engine directly with:
More commands and preprocessor calls to be added:
```bash
cmake -B build
cmake --build build
