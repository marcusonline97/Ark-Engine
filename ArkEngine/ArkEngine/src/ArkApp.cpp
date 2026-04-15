#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "ArkApp.h"
#include "ArkWindow.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <limits>

#include "Logger.h"
#include "Utility/Utility.h"
#include "Utility/SceneIO.h"
#include "Camera/CameraController.h"
#include "Input/Input.h"

static constexpr const char* kImGuiGLSLVersion = "#version 450";

namespace
{
	constexpr int kResourcesPerFrame = 2;
	constexpr size_t kInvalidCameraIndex = std::numeric_limits<size_t>::max();

	Ark::CameraInput BuildCameraInputFromKeyboard()
	{
		Ark::CameraInput input{};
		input.forward = Ark::Input::IsKeyDown(ARK_KEY_W);
		input.back = Ark::Input::IsKeyDown(ARK_KEY_S);
		input.left = Ark::Input::IsKeyDown(ARK_KEY_A);
		input.right = Ark::Input::IsKeyDown(ARK_KEY_D);
		input.up = Ark::Input::IsKeyDown(ARK_KEY_E);
		input.down = Ark::Input::IsKeyDown(ARK_KEY_Q);
		input.fast = Ark::Input::IsKeyDown(ARK_KEY_LEFT_SHIFT);
		return input;
	}

	glm::mat4 BuildModelMatrix(const EditorObject& obj)
	{
		const glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(obj.rotationDeg.x), glm::vec3(1, 0, 0));
		const glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(obj.rotationDeg.y), glm::vec3(0, 1, 0));
		const glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), glm::radians(obj.rotationDeg.z), glm::vec3(0, 0, 1));
		const glm::mat4 rot = rotZ * rotY * rotX;
		return glm::translate(glm::mat4(1.0f), obj.position) * rot * glm::scale(glm::mat4(1.0f), obj.scale);
	}

	bool IsSameVec3(const glm::vec3& a, const glm::vec3& b)
	{
		return a.x == b.x && a.y == b.y && a.z == b.z;
	}

}

// TODO(camera-legacy): BasicCamera and ViewportCamera still exist in the tree but
// runtime control now goes through CameraController-based possession/editor logic.

GLFWwindow* App::GetWindowHandle() const
{
	return m_Window ? m_Window->GetNativeHandle() : nullptr;

}

App::App()
{
	m_Window = std::make_unique<ArkWindow>(1400, 840, "Ark Engine");
	Ark::Input::SetWindow(m_Window->GetNativeHandle());

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		throw std::runtime_error("Failed to initialize GLAD");

	glEnable(GL_DEPTH_TEST);

	glfwSetFramebufferSizeCallback(
		m_Window->GetNativeHandle(),
		[](GLFWwindow*, int width, int height)
		{
			glViewport(0, 0, width, height);
		}
	);

	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

	const bool imguiOk = m_ImGui.Init(m_Window->GetNativeHandle(), kImGuiGLSLVersion);
	Logging::ToDo() << "Initializing ImGui.\n";

	if (imguiOk)
	{
		m_EditorUI.Init();
		m_LogSinkId = Logging::AddSink([this](Logging::Level lvl, std::string_view msg)
			{
				m_EditorUI.PushLog(lvl, msg);
			});
	}

	// Load blank map first (clean state), then load EditorScene.json.
	{
		// Blank map = no objects
		m_Objects.clear();
		m_SelectedObject = -1;

		const std::filesystem::path scenePath =
			std::filesystem::current_path() / "ArkEngine" / "Resources" / "Scenes" / "EditorScene.json";

		if (!Ark::Editor::LoadEditorScene(scenePath, m_Objects))
		{
			Logging::Warning() << "Failed to load scene: " << scenePath.string() << " (scene will be empty)\n";

			// IMPORTANT: Do not create fallback objects here.
			// The Hierarchy should reflect only what's in the Scene.json file.
			m_Objects.clear();
		}

		m_SelectedObject = m_Objects.empty() ? -1 : 0;
	}

	m_WorldRenderer = std::make_unique<Ark::Rendering::WorldRenderThread>(
		m_Window->GetNativeHandle(),
		&m_cpuResourceLoader);
}

App::~App()
{
	if (m_LogSinkId != 0)
	{
		Logging::RemoveSink(m_LogSinkId);
		m_LogSinkId = 0;
	}

	m_EditorUI.Shutdown();
	m_ImGui.Shutdown();
	m_WorldRenderer.reset();
}

void App::Run()
{
	double lastTime = glfwGetTime();
	bool rotating = false;
	double lastMouseX = 0.0;
	double lastMouseY = 0.0;

	bool showGrid = true;

	{
		std::uint32_t nextId = 1;
		for (auto& o : m_Objects)
		{
			if (o.id == 0)
				o.id = nextId++;
			else
				nextId = std::max(nextId, o.id + 1);
		}
	}

	while (!m_Window->ShouldClose())
	{
		const double now = glfwGetTime();
		const float dt = static_cast<float>(now - lastTime);
		lastTime = now;

		Ark::Input::NewFrame();

		auto isValidCameraIndex = [this](size_t index)
			{
				return index < m_Objects.size() &&
					m_Objects[index].enabled &&
					m_Objects[index].camera.has_value();
			};

		auto refreshPrimaryCameraIndex = [this, &isValidCameraIndex]()
			{
				size_t firstCamera = kInvalidCameraIndex;
				size_t primaryCamera = kInvalidCameraIndex;

				for (size_t i = 0; i < m_Objects.size(); ++i)
				{
					const EditorObject& o = m_Objects[i];
					if (!o.enabled || !o.camera)
						continue;

					if (firstCamera == kInvalidCameraIndex)
						firstCamera = i;

					if (o.camera->primary)
					{
						primaryCamera = i;
						break;
					}
				}

				m_hasExplicitPrimaryCamera = primaryCamera != kInvalidCameraIndex;
				m_primaryCameraIndex = m_hasExplicitPrimaryCamera ? primaryCamera : firstCamera;

				if (!isValidCameraIndex(m_primaryCameraIndex))
					m_primaryCameraIndex = kInvalidCameraIndex;
			};

		refreshPrimaryCameraIndex();

		if (Ark::Input::IsKeyPressed(ARK_KEY_G))
		{
			showGrid = !showGrid;

			if (!showGrid)
				Logging::Debug() << "Grid cleared (hidden).\n";
			else
				Logging::Debug() << "Grid enabled.\n";
		}

		// CPU iterative resource loading: do a small bounded amount per frame.
		m_cpuResourceLoader.Pump(kResourcesPerFrame);

		// Play mode: possess the primary camera and drive it with basic FPS controls.
		if (m_EditorUI.IsPlayMode())
		{
			if (isValidCameraIndex(m_primaryCameraIndex))
			{
				EditorObject& camObj = m_Objects[m_primaryCameraIndex];
				Ark::CameraController cameraController{};
				cameraController.position = camObj.position;
				cameraController.pitchDeg = camObj.rotationDeg.x;
				cameraController.yawDeg = camObj.rotationDeg.y;
				cameraController.moveSpeed = 3.5f;
				cameraController.lookSensitivity = 0.12f;
				cameraController.ProcessKeyboard(dt, BuildCameraInputFromKeyboard());

				// Hold RMB to rotate the camera.
				if (Ark::Input::IsMouseDown(ARK_MOUSE_RIGHT))
				{
					double mx = 0.0, my = 0.0;
					glfwGetCursorPos(m_Window->GetNativeHandle(), &mx, &my);
					if (!rotating)
					{
						rotating = true;
						lastMouseX = mx;
						lastMouseY = my;
					}
					else
					{
						const double dx = mx - lastMouseX;
						const double dy = my - lastMouseY;
						lastMouseX = mx;
						lastMouseY = my;

						cameraController.ProcessMouseDelta(static_cast<float>(dx), static_cast<float>(dy));
					}
				}
				else
				{
					rotating = false;
				}

				camObj.position = cameraController.position;
				camObj.rotationDeg.x = cameraController.pitchDeg;
				camObj.rotationDeg.y = cameraController.yawDeg;
			}
			else
			{
				rotating = false;
			}
		}

		glm::vec2 vpSize = m_EditorUI.GetViewportSize();
		int vpW = static_cast<int>(vpSize.x);
		int vpH = static_cast<int>(vpSize.y);
		if (vpW < 16 || vpH < 16)
			glfwGetFramebufferSize(m_Window->GetNativeHandle(), &vpW, &vpH);

		if (vpW < 1) vpW = 1;
		if (vpH < 1) vpH = 1;

		if (m_WorldRenderer)
		{
			Ark::Rendering::WorldRenderInput& input = m_renderInput;
			input.width = static_cast<uint32_t>(vpW);
			input.height = static_cast<uint32_t>(vpH);

			input.wireframe = m_EditorUI.GetWireframeEnabled();
			input.useMipmaps = m_EditorUI.GetUseMipmaps();
			input.showGrid = m_EditorUI.GetShowGrid();

			// Camera selection:
			// - EDIT mode: use the editor viewport camera (world-space).
			// - PLAY mode: use the scene's primary camera (or first camera).
			if (!m_EditorUI.IsPlayMode())
			{
				input.camera = m_EditorUI.GetEditorViewportCamera();
			}
			else
			{
				if (isValidCameraIndex(m_primaryCameraIndex))
				{
					const EditorObject& camObj = m_Objects[m_primaryCameraIndex];
					input.camera.position = camObj.position;
					input.camera.pitchYawDeg = glm::vec2(camObj.rotationDeg.x, camObj.rotationDeg.y);
					input.camera.fovDeg = camObj.camera->fovDeg;
					input.camera.nearPlane = camObj.camera->nearPlane;
					input.camera.farPlane = camObj.camera->farPlane;
				}
			}

			input.instances.clear();
			input.instances.reserve(m_Objects.size());
			input.pointLights.clear();
			input.pointLights.reserve(m_Objects.size());

			if (m_cachedObjectTransforms.size() != m_Objects.size())
				m_cachedObjectTransforms.resize(m_Objects.size());
			if (m_cachedObjectModels.size() != m_Objects.size())
				m_cachedObjectModels.resize(m_Objects.size(), glm::mat4(1.0f));

			for (size_t objectIndex = 0; objectIndex < m_Objects.size(); ++objectIndex)
			{
				const auto& o = m_Objects[objectIndex];
				if (!o.enabled)
					continue;

				if (o.staticMesh || o.skeletalMesh)
				{
					Ark::Rendering::RenderInstance inst{};
					inst.objectId = o.id;

					CachedTransformState& cachedState = m_cachedObjectTransforms[objectIndex];
					const bool transformChanged =
						cachedState.objectId != o.id ||
						!IsSameVec3(cachedState.position, o.position) ||
						!IsSameVec3(cachedState.rotation, o.rotationDeg) ||
						!IsSameVec3(cachedState.scale, o.scale);

					if (transformChanged)
					{
						cachedState.objectId = o.id;
						cachedState.position = o.position;
						cachedState.rotation = o.rotationDeg;
						cachedState.scale = o.scale;
						m_cachedObjectModels[objectIndex] = BuildModelMatrix(o);
					}

					inst.model = m_cachedObjectModels[objectIndex];
					inst.tint = o.tint;

					inst.hasMaterial = false;
					if (o.materialPreset != 0)
						inst.hasMaterial = true;

					if (o.staticMesh && !o.staticMesh->texturePath.empty())
						inst.hasMaterial = true;
					if (o.skeletalMesh && !o.skeletalMesh->texturePath.empty())
						inst.hasMaterial = true;

					if (o.staticMesh)
					{
						inst.meshType = Ark::Rendering::RenderMeshType::Static;

						if (!o.staticMesh->meshPath.empty())
							inst.meshPath = o.staticMesh->meshPath;

						if (!o.staticMesh->texturePath.empty())
							inst.albedoTexturePath = o.staticMesh->texturePath;
					}
					else if (o.skeletalMesh)
					{
						inst.meshType = Ark::Rendering::RenderMeshType::Skeletal;

						if (!o.skeletalMesh->meshPath.empty())
							inst.meshPath = o.skeletalMesh->meshPath;

						if (!o.skeletalMesh->texturePath.empty())
							inst.albedoTexturePath = o.skeletalMesh->texturePath;

						inst.animationPath = o.skeletalMesh->animationPath;
						inst.animationIndex = o.skeletalMesh->animationIndex;
					}

					input.instances.push_back(inst);
				}

				// Scene light (affects shading)
				if (o.pointLight)
				{
					Ark::Rendering::PointLightInput l{};
					l.position = o.position;
					l.color = o.pointLight->color;
					l.intensity = o.pointLight->intensity;
					l.radius = o.pointLight->radius;
					input.pointLights.push_back(l);

					// NOTE: Do NOT render point lights as proxy cubes. Gizmos are handled by ImGuizmo in the editor UI.
				}
			}

			m_WorldRenderer->Submit(input);
		}

		Utilities::TickViewportFPS(glfwGetTime());

		// Clear the main framebuffer before drawing UI.
		int winW = 0, winH = 0;
		glfwGetFramebufferSize(m_Window->GetNativeHandle(), &winW, &winH);
		glViewport(0, 0, winW, winH);
		glClearColor(0.12f, 0.12f, 0.13f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (m_ImGui.IsInitialized())
		{
			m_ImGui.BeginFrame();
			m_EditorUI.SetViewportTextureId(m_WorldRenderer ? m_WorldRenderer->GetLatestTextureId() : 0);
			m_EditorUI.SetViewportTriangleCount(m_WorldRenderer ? m_WorldRenderer->GetLatestTriangleCount() : 0);
			m_EditorUI.Render(m_Objects, m_SelectedObject);

			const std::filesystem::path scenePath =
				std::filesystem::current_path() / "ArkEngine" / "Resources" / "Scenes" / "EditorScene.json";

			if (m_EditorUI.ConsumeSaveSceneRequested())
			{
				if (Ark::Editor::SaveEditorScene(scenePath, m_Objects))
					Logging::Debug() << "Scene saved: " << scenePath.string() << "\n";
			}

			if (m_EditorUI.ConsumeLoadSceneRequested())
			{
				if (Ark::Editor::LoadEditorScene(scenePath, m_Objects))
				{
					m_SelectedObject = m_Objects.empty() ? -1 : 0;
					Logging::Debug() << "Scene loaded: " << scenePath.string() << "\n";
				}
			}
			m_ImGui.EndFrame();
		}

		m_Window->SwapBuffers();
		m_Window->PollEvents();
	}
}