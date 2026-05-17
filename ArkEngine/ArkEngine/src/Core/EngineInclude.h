#pragma once

#include "Application.h"
#include "ArkEngine.h"
#include "Input/InputManager.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/GraphicsAPI.h"
#include "Graphics/VertexLayout.h"
#include "Graphics/Texture.h"
#include "Render/Material.h"
#include "Render/Mesh.h"
#include "Render/RenderQueue.h"
#include "Scene/Scene.h"
#include "Scene/GameObject.h"
#include "Scene/Component.h"
#include "Scene/Components/MeshComponent.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/PlayerControllerComponent.h"
#include "Scene/Components/LightComponent.h"
#include "AssetManager/FileSystem.h"

// This header file serves as a central include for all core engine components, allowing other parts of the codebase to include this single file to access the entire engine functionality.