# ArkEngine

ArkEngine — a custom 3D game engine written in C++

![Editor Screenshot](docs/screenshot.png)

## Features

**Rendering**
- OpenGL 3.3 Core rendering through GLFW and GLAD.
- Scene rendering into a framebuffer object for editor viewport display or screen blitting.
- Optional deferred rendering path with geometry, shadow, and lighting passes.
- G-buffer attachments for world position, normal, and albedo/specular data.
- Directional and point light data collection from the scene graph.
- Shadow mapping for directional lights with a 2048x2048 depth map and 3x3 PCF filtering.
- Specular lighting with global specular strength control and optional `specularMap` material texture.
- Forward rendering fallback through the default material shader path.
- Runtime wireframe mode, depth test, viewport, clear color, and blend mode controls.
- 3D mesh, 2D sprite, and UI draw submission through `RenderQueue`.
- Runtime mipmap filter control for loaded asset textures.

**Editor**
- In-engine Dear ImGui editor with docking enabled.
- Docked Hierarchy, Viewport, Inspector, and Bottom Panel layout.
- Bottom Panel tabs for Content Browser, Rendering, and Logs.
- Viewport panel renders the engine scene FBO.
- Hierarchy supports selecting objects, creating children, duplicating objects, and deleting objects.
- Inspector supports object active state, name, transform editing, component listing, component removal, and registered component creation.
- Rendering tab exposes deferred rendering, mipmap filter, and specular strength controls.
- Logs tab captures engine log messages with level filters and auto-scroll.
- Menu, Play, and Edit application modes.
- Edit mode uses an editor fly camera.

**Scene System**
- `Scene` owns root `GameObject` instances and supports parent/child hierarchy changes.
- `GameObject` supports active state, lifetime marking, local/world transforms, components, and serialized JSON data.
- Component factory registration for cameras, lights, meshes, physics, audio, sprites, UI, animation, and player controller components.
- Scene save/load through nlohmann/json.
- Scene main camera selection by serialized camera object name.
- Active UI canvas lookup by serialized canvas object name.
- GLTF scene/object loading through `GameObject::LoadGLTF`.
- Transform animation clips with translation, rotation, and scale tracks.

**Physics**
- Bullet Physics integration through `PhysicsManager`.
- Bullet broadphase, collision configuration, dispatcher, solver, and discrete dynamics world ownership.
- Rigid body add/remove and per-frame physics updates.
- Physics component, colliders, rigid bodies, collision objects, and kinematic character controller are public engine systems.

**Audio**
- miniaudio engine ownership through `AudioManager`.
- Listener position updates.
- Audio and audio listener components are public engine systems.

**UI**
- Dear ImGui editor UI with docking.
- Runtime UI systems for canvas, text, UI element, button, rect transform, and UI input components.
- UI render submission supports batched indexed drawing with optional textures.
- FreeType-backed font manager with font caching by path and size.

**Asset Pipeline**
- Asset file access through the engine file system.
- GLTF/GLB loading through cgltf, including node transforms, mesh primitives, material textures, and animation clips.
- Assimp mesh loading fallback for non-GLTF mesh files.
- Content Browser recognizes directories, images, materials, scenes, audio files, and mesh files.
- Mesh assets can be drag-dropped from the Content Browser onto mesh components.
- Materials load from JSON with shader paths, float/float2/float3 parameters, and texture parameters.
- Scene files use `.sc` JSON; material files use `.mat` JSON.

## Tech Stack

| Library | Purpose |
| --- | --- |
| Language | C++17 |
| Graphics API | OpenGL 3.3 Core |
| Windowing | GLFW |
| Extension loading | GLAD |
| Math | GLM |
| UI | Dear ImGui with docking |
| Physics | Bullet Physics |
| Audio | miniaudio |
| Model loading | cgltf, Assimp |
| Font rendering | FreeType |
| JSON | nlohmann/json |

## Architecture Overview

`ArkEngine` is a singleton that owns the main application, window, input manager, graphics API, render queue, deferred renderer, file system, texture manager, mesh manager, physics manager, audio manager, font manager, UI input system, and current scene. The core loop updates physics and UI input, updates the application, renders the scene into an FBO or deferred output, draws the editor UI, then swaps the GLFW buffers.

Scenes are built from a `Scene` / `GameObject` / `Component` hierarchy. A scene owns root objects, objects can own children and components, and registered object/component factories recreate serialized types during load. Scenes collect lights recursively and resolve the serialized main camera and active canvas by object name.

Rendering is submitted through `RenderQueue` commands for 3D meshes, 2D sprites, and UI batches. The engine can render queued 3D commands through the deferred renderer first, then draw remaining 2D/UI commands, or use the forward path directly when deferred rendering is disabled.

## Getting Started

### Prerequisites

<!-- TODO: fill in prerequisites -->

### Building

<!-- TODO: fill in build instructions. CMake is assumed. -->

### Running

<!-- TODO: fill in run instructions -->

## Scene File Format

Scenes are stored as `.sc` JSON files. Materials are stored as `.mat` JSON files and can reference shader files, numeric parameters, and textures.

Basic scene shape:

```json
{
  "name": "ExampleScene",
  "camera": "MainCamera",
  "activeCanvas": "HUDCanvas",
  "objects": [
    {
      "name": "MainCamera",
      "position": { "x": 0.0, "y": 2.0, "z": 5.0 },
      "rotation": { "x": 0.0, "y": 0.0, "z": 0.0, "w": 1.0 },
      "scale": { "x": 1.0, "y": 1.0, "z": 1.0 },
      "components": [
        { "type": "CameraComponent" }
      ]
    },
    {
      "name": "Sun",
      "position": { "x": 0.0, "y": 4.0, "z": 0.0 },
      "rotation": { "x": 0.0, "y": 0.0, "z": 0.0, "w": 1.0 },
      "scale": { "x": 1.0, "y": 1.0, "z": 1.0 },
      "components": [
        {
          "type": "directional",
          "componentType": "LightComponent",
          "color": { "r": 1.0, "g": 1.0, "b": 1.0 },
          "intensity": 1.0,
          "range": 10.0
        }
      ]
    },
    {
      "name": "Model",
      "type": "gltf",
      "path": "Models/model.gltf",
      "position": { "x": 0.0, "y": 0.0, "z": 0.0 },
      "rotation": { "x": 0.0, "y": 0.0, "z": 0.0, "w": 1.0 },
      "scale": { "x": 1.0, "y": 1.0, "z": 1.0 },
      "children": [
        {
          "name": "Child",
          "position": { "x": 1.0, "y": 0.0, "z": 0.0 },
          "rotation": { "x": 0.0, "y": 0.0, "z": 0.0, "w": 1.0 },
          "scale": { "x": 1.0, "y": 1.0, "z": 1.0 }
        }
      ]
    }
  ]
}
```

## Editor Usage

The application starts in Menu mode with UI buttons for Play, Edit, and Quit. Play mode disables the menu canvas, enables the game scene, restores the game camera, and locks the cursor. Edit mode disables gameplay/player control, enables the docked editor, switches the main camera to the editor camera, and renders the scene in the editor viewport. Escape returns from Play or Edit mode to Menu mode.

Editor layout:
- Hierarchy: scene object tree and object context actions.
- Viewport: rendered scene FBO, save button, add-object control, scene path, and camera hints.
- Inspector: selected object state, transform, components, and add/remove controls.
- Bottom Panel: Content Browser, Rendering, and Logs tabs.

Editor camera controls:
- WASD: fly movement.
- RMB: look.
- E or Space: move up.
- Q or Ctrl: move down.
- Shift: fast movement.

## License

<!-- TODO: add license -->
