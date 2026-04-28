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
#include "Input/Input.h"
#include "ArkPhysics.h"
#include "AssetManager.h"

static constexpr const char* kImGuiGLSLVersion = "#version 450";

namespace
{
	// Resolve scene path through the AssetManager so project-relative paths work correctly.
	std::filesystem::path ResolveEditorScenePath()
	{
		return AssetManager::Instance().ResolveAssetPath("Resources/Scenes/EditorScene.json");
	}

	// Build a TRS model matrix from an EditorObject's transform components.
	// Rotation order: Z * Y * X (applied right to left, i.e. X first).
	glm::mat4 BuildModelMatrix(const EditorObject& obj)
	{
		const glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(obj.rotationDeg.x), glm::vec3(1, 0, 0));
		const glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(obj.rotationDeg.y), glm::vec3(0, 1, 0));
		const glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), glm::radians(obj.rotationDeg.z), glm::vec3(0, 0, 1));
		return glm::translate(glm::mat4(1.0f), obj.position)
			* (rotZ * rotY * rotX)
			* glm::scale(glm::mat4(1.0f), obj.scale);
	}

	// Find the first enabled camera object that is marked primary,
	// falling back to the first camera if none is marked primary.
	EditorObject* FindActiveCamera(std::vector<EditorObject>& objects)
	{
		EditorObject* fallback = nullptr;
		for (auto& o : objects)
		{
			if (!o.enabled || !o.camera)
				continue;
			if (o.camera->primary)
				return &o;
			if (!fallback)
				fallback = &o;
		}
		return fallback;
	}
}

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
		});

	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";

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

	// Load a clean state first, then load the saved scene.
	{
		m_Objects.clear();
		m_SelectedObject = -1;

		const std::filesystem::path scenePath = ResolveEditorScenePath();
		if (!Ark::Editor::LoadEditorScene(scenePath, m_Objects))
		{
			Logging::Warning() << "Failed to load scene: " << scenePath.string() << " (scene will be empty)\n";
			m_Objects.clear();
		}

		m_SelectedObject = m_Objects.empty() ? -1 : 0;
	}

	m_WorldRenderer = std::make_unique<Ark::Rendering::WorldRenderThread>(
		m_Window->GetNativeHandle(),
		&m_cpuResourceLoader);
	m_PhysicsWorld = std::make_unique<Ark::Physics::PhysicsThreadedWorld>();
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
	m_PhysicsWorld.reset();
}

void App::Run()
{
	double lastTime = glfwGetTime();

	// Assign stable IDs to any objects that were loaded without one.
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

		// --- Physics: consume latest transforms and apply to dynamic objects ---
		if (m_PhysicsWorld && m_EditorUI.IsPlayMode())
		{
			const auto physicsTransforms = m_PhysicsWorld->ConsumeLatestTransforms();
			for (const auto& t : physicsTransforms)
			{
				if (t.objectId == 0)
					continue;

				for (auto& o : m_Objects)
				{
					if (o.id != t.objectId || !o.physicsBody || !o.enabled)
						continue;

					const int motionType = std::clamp(o.physicsBody->motionType, 0, 2);
					if (motionType == 1) // Dynamic
					{
						o.position = t.position;
						o.rotationDeg = t.rotationDeg;
					}
					break;
				}
			}
		}

		// --- CPU resource streaming ---
		m_cpuResourceLoader.Pump(2);

		// --- Physics: submit current scene state ---
		if (m_PhysicsWorld)
		{
			std::vector<Ark::Physics::BodyConfig> physicsBodies;
			physicsBodies.reserve(m_Objects.size());

			for (const auto& o : m_Objects)
			{
				if (!o.enabled || !o.physicsBody || o.id == 0)
					continue;

				Ark::Physics::BodyConfig cfg{};
				cfg.objectId = o.id;
				cfg.motionType = static_cast<Ark::Physics::MotionType>(std::clamp(o.physicsBody->motionType, 0, 2));
				cfg.useGravity = o.physicsBody->useGravity;
				cfg.position = o.position;
				cfg.rotationDeg = o.rotationDeg;

				const glm::vec3 safeHalf = glm::max(glm::abs(o.physicsBody->halfExtents), glm::vec3(0.01f));
				cfg.halfExtents = glm::vec3(
					safeHalf.x * std::max(0.01f, std::abs(o.scale.x)),
					safeHalf.y * std::max(0.01f, std::abs(o.scale.y)),
					safeHalf.z * std::max(0.01f, std::abs(o.scale.z)));

				physicsBodies.push_back(cfg);
			}

			m_PhysicsWorld->SetSimulationEnabled(m_EditorUI.IsPlayMode());
			m_PhysicsWorld->SubmitScene(physicsBodies);
		}

		// --- Play-mode: drive the primary scene camera with FPS controls ---
		// Note: edit-mode camera is fully managed by EditorUI (possession/viewport).
		if (m_EditorUI.IsPlayMode())
		{
			if (EditorObject* camObj = FindActiveCamera(m_Objects))
			{
				const float pitch = camObj->rotationDeg.x;
				const float yaw = camObj->rotationDeg.y;

				glm::vec3 front;
				front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
				front.y = sin(glm::radians(pitch));
				front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
				front = glm::normalize(front);

				const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
				const glm::vec3 right = glm::normalize(glm::cross(worldUp, front));
				const float move = 3.5f * dt;

				if (Ark::Input::IsKeyDown(ARK_KEY_W)) camObj->position += front * move;
				if (Ark::Input::IsKeyDown(ARK_KEY_S)) camObj->position -= front * move;
				if (Ark::Input::IsKeyDown(ARK_KEY_A)) camObj->position -= right * move;
				if (Ark::Input::IsKeyDown(ARK_KEY_D)) camObj->position += right * move;
				if (Ark::Input::IsKeyDown(ARK_KEY_E)) camObj->position += worldUp * move;
				if (Ark::Input::IsKeyDown(ARK_KEY_Q)) camObj->position -= worldUp * move;

				// RMB look
				static bool rotating = false;
				static double lastMouseX = 0.0;
				static double lastMouseY = 0.0;

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
						constexpr float sensitivity = 0.12f;
						camObj->rotationDeg.y += static_cast<float>(mx - lastMouseX) * sensitivity;
						camObj->rotationDeg.x -= static_cast<float>(my - lastMouseY) * sensitivity;
						camObj->rotationDeg.x = std::clamp(camObj->rotationDeg.x, -89.0f, 89.0f);
						lastMouseX = mx;
						lastMouseY = my;
					}
				}
				else
				{
					rotating = false;
				}
			}
		}

		// --- Resolve viewport dimensions ---
		glm::vec2 vpSize = m_EditorUI.GetViewportSize();
		int vpW = static_cast<int>(vpSize.x);
		int vpH = static_cast<int>(vpSize.y);
		if (vpW < 16 || vpH < 16)
			glfwGetFramebufferSize(m_Window->GetNativeHandle(), &vpW, &vpH);
		vpW = std::max(vpW, 1);
		vpH = std::max(vpH, 1);

		// --- Build and submit render input ---
		if (m_WorldRenderer)
		{
			Ark::Rendering::WorldRenderInput input{};
			input.width = static_cast<uint32_t>(vpW);
			input.height = static_cast<uint32_t>(vpH);
			input.wireframe = m_EditorUI.GetWireframeEnabled();
			input.useMipmaps = m_EditorUI.GetUseMipmaps();
			input.showGrid = m_EditorUI.GetShowGrid();

			// Camera selection: edit mode uses the editor viewport camera,
			// play mode drives from the primary scene camera object.
			if (!m_EditorUI.IsPlayMode())
			{
				input.camera = m_EditorUI.GetEditorViewportCamera();
			}
			else if (EditorObject* camObj = FindActiveCamera(m_Objects))
			{
				input.camera.position = camObj->position;
				input.camera.pitchYawDeg = glm::vec2(camObj->rotationDeg.x, camObj->rotationDeg.y);
				input.camera.fovDeg = camObj->camera ? camObj->camera->fovDeg : 45.0f;
				input.camera.nearPlane = camObj->camera ? camObj->camera->nearPlane : 0.1f;
				input.camera.farPlane = camObj->camera ? camObj->camera->farPlane : 100.0f;
			}
			m_EditorUI.SetViewportRenderCamera(input.camera);

			// Build instance and light lists from the enabled scene objects.
			input.instances.reserve(m_Objects.size());
			input.pointLights.reserve(m_Objects.size());

			const auto resolve = [](const std::string& p) -> std::string
				{
					return p.empty() ? std::string{} : AssetManager::Instance().ResolveAssetPath(p);
				};

			for (const auto& o : m_Objects)
			{
				if (!o.enabled)
					continue;

				if (o.staticMesh || o.skeletalMesh)
				{
					Ark::Rendering::RenderInstance inst{};
					inst.objectId = o.id;
					inst.model = BuildModelMatrix(o);
					inst.tint = o.tint;
					inst.hasMaterial = (o.materialPreset != 0);

					if (o.staticMesh)
					{
						inst.meshType = Ark::Rendering::RenderMeshType::Static;
						inst.meshPath = resolve(o.staticMesh->meshPath);
						inst.albedoTexturePath = resolve(o.staticMesh->texturePath);
						if (!inst.albedoTexturePath.empty())
							inst.hasMaterial = true;
					}
					else if (o.skeletalMesh)
					{
						inst.meshType = Ark::Rendering::RenderMeshType::Skeletal;
						inst.meshPath = resolve(o.skeletalMesh->meshPath);
						inst.albedoTexturePath = resolve(o.skeletalMesh->texturePath);
						inst.animationPath = resolve(o.skeletalMesh->animationPath);
						inst.animationIndex = o.skeletalMesh->animationIndex;
						if (!inst.albedoTexturePath.empty())
							inst.hasMaterial = true;
					}

					input.instances.push_back(inst);
				}

				if (o.pointLight)
				{
					Ark::Rendering::PointLightInput l{};
					l.position = o.position;
					l.color = o.pointLight->color;
					l.intensity = o.pointLight->intensity;
					l.radius = o.pointLight->radius;
					input.pointLights.push_back(l);
				}
			}

			m_WorldRenderer->Submit(input);
		}

		Utilities::TickViewportFPS(glfwGetTime());

		// --- Clear main framebuffer, then render UI on top ---
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

			const std::filesystem::path scenePath = ResolveEditorScenePath();

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