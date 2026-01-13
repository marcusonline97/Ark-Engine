#pragma once

#include <string_view>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "ImguiStyling.h"
#include "Logger.h"

struct GLFWwindow;

namespace Ark::Editor
{
	// Small RAII wrapper for ImGui context + backends.
	// Keeps ArkApp focused on "app flow" rather than ImGui lifecycle details.
	class ImGuiLayer
	{
	public:
		ImGuiLayer() = default;
		~ImGuiLayer() { Shutdown(); }

		ImGuiLayer(const ImGuiLayer&) = delete;
		ImGuiLayer& operator=(const ImGuiLayer&) = delete;

		bool Init(GLFWwindow* windowHandle, std::string_view glslVersion)
		{
			if (m_initialized)
				return true;

			if (!windowHandle)
			{
				Logging::Error() << "ImGuiLayer::Init called with null window handle.\n";
				return false;
			}

			IMGUI_CHECKVERSION();
			ImGui::CreateContext();

			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

			Ark::EditorTheme::ApplyDarkTheme();

			if (!ImGui_ImplGlfw_InitForOpenGL(windowHandle, true))
			{
				Logging::Error() << "ImGui_ImplGlfw_InitForOpenGL failed.\n";
				Shutdown();
				return false;
			}

			// Initialize OpenGL3 backend with GLSL version.
			if (!ImGui_ImplOpenGL3_Init(glslVersion.data()))
			{
				Logging::Error() << "ImGui_ImplOpenGL3_Init failed.\n";
				Shutdown();
				return false;
			}

			m_initialized = true;
			return true;
		}

		void Shutdown()
		{
			if (!m_initialized)
				return;

			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			m_initialized = false;
		}

		void BeginFrame() const
		{
			if (!m_initialized)
				return;

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}

		void EndFrame() const
		{
			if (!m_initialized)
				return;

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		bool IsInitialized() const { return m_initialized; }

	private:
		bool m_initialized = false;
	};
}

