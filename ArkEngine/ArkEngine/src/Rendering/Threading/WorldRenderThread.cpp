#include "WorldRenderThread.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "AssetManager.h"
#include "Camera/Camera.h"
#include "Logger.h"
#include "Meshes/Cube.h"
#include "Rendering/Framebuffer/Framebuffer.h"
#include "Shader.h"

#include <glm/gtc/matrix_transform.hpp>

#if !defined(ARK_HAS_ASSIMP)
// CMake defines this when it links assimp. For VS builds we default-on (the .vcxproj links Assimp).
	#if defined(_WIN32)
		#define ARK_HAS_ASSIMP 1
	#else
		#define ARK_HAS_ASSIMP 0
	#endif
#endif

#if ARK_HAS_ASSIMP
	#include <Importer.hpp>
	#include <postprocess.h>
	#include <scene.h>
#endif

namespace Ark::Rendering
{

	struct ObjGpuMesh
	{
		~ObjGpuMesh()
		{
			if (m_vao) glDeleteVertexArrays(1, &m_vao);
			if (m_vbo) glDeleteBuffers(1, &m_vbo);
		}

		bool LoadFromObj(const std::filesystem::path& path)
		{
			std::ifstream f(path);
			if (!f.is_open())
				return false;

			std::vector<glm::vec3> positions;
			positions.reserve(1024);

			std::vector<glm::vec3> triPositions;
			triPositions.reserve(4096);

			auto parseIndex = [](const std::string& token, int posCount) -> int
				{
					// token can be: "v", "v/vt", "v//vn", "v/vt/vn"
					// We only care about vertex index (v).
					const size_t slash = token.find('/');
					const std::string vstr = (slash == std::string::npos) ? token : token.substr(0, slash);
					if (vstr.empty())
						return -1;

					int idx = std::stoi(vstr);
					// OBJ indices are 1-based; negative means relative to end.
					if (idx > 0) idx = idx - 1;
					else if (idx < 0) idx = posCount + idx;
					else idx = -1;
					return idx;
				};

			std::string line;
			while (std::getline(f, line))
			{
				if (line.empty())
					continue;
				if (line.size() >= 2 && line[0] == 'v' && std::isspace(static_cast<unsigned char>(line[1])))
				{
					std::istringstream iss(line);
					char v = 0;
					float x = 0, y = 0, z = 0;
					iss >> v >> x >> y >> z;
					positions.emplace_back(x, y, z);
					continue;
				}
				if (line.size() >= 2 && line[0] == 'f' && std::isspace(static_cast<unsigned char>(line[1])))
				{
					std::istringstream iss(line);
					char fch = 0;
					iss >> fch;

					std::vector<int> face;
					face.reserve(8);

					std::string tok;
					while (iss >> tok)
					{
						const int idx = parseIndex(tok, static_cast<int>(positions.size()));
						if (idx >= 0 && idx < static_cast<int>(positions.size()))
							face.push_back(idx);
					}

					// Triangulate fan: (0, i, i+1)
					if (face.size() >= 3)
					{
						for (size_t i = 1; i + 1 < face.size(); ++i)
						{
							triPositions.push_back(positions[static_cast<size_t>(face[0])]);
							triPositions.push_back(positions[static_cast<size_t>(face[i])]);
							triPositions.push_back(positions[static_cast<size_t>(face[i + 1])]);
						}
					}
				}
			}

			if (triPositions.empty())
				return false;

			// Normalize to fit inside [-0.5, 0.5] cube for consistent preview scale.
			glm::vec3 mn = triPositions[0];
			glm::vec3 mx = triPositions[0];
			for (const auto& p : triPositions)
			{
				mn = glm::min(mn, p);
				mx = glm::max(mx, p);
			}
			const glm::vec3 center = 0.5f * (mn + mx);
			const glm::vec3 ext = (mx - mn);
			const float maxExtent = std::max(ext.x, std::max(ext.y, ext.z));
			const float inv = (maxExtent > 0.000001f) ? (1.0f / maxExtent) : 1.0f;

			std::vector<float> verts;
			verts.reserve(triPositions.size() * 6);
			for (auto p : triPositions)
			{
				p = (p - center) * inv; // now roughly within [-0.5, 0.5]
				verts.push_back(p.x);
				verts.push_back(p.y);
				verts.push_back(p.z);
				// vertex color (white) - multiplied by u_Tint in shader
				verts.push_back(1.0f);
				verts.push_back(1.0f);
				verts.push_back(1.0f);
			}

			m_vertexCount = static_cast<GLsizei>(triPositions.size());

			if (!m_vao) glGenVertexArrays(1, &m_vao);
			if (!m_vbo) glGenBuffers(1, &m_vbo);

			glBindVertexArray(m_vao);
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
			glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(verts.size() * sizeof(float)), verts.data(), GL_STATIC_DRAW);

			// position
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			// color
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			return true;
		}

		void Draw() const
		{
			glBindVertexArray(m_vao);
			glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
		}

		GLuint m_vao = 0;
		GLuint m_vbo = 0;
		GLsizei m_vertexCount = 0;
	};

#if ARK_HAS_ASSIMP
	struct AssimpGpuMesh
	{
		~AssimpGpuMesh()
		{
			if (m_vao) glDeleteVertexArrays(1, &m_vao);
			if (m_vbo) glDeleteBuffers(1, &m_vbo);
		}

		bool LoadFromFile(const std::filesystem::path& path)
		{
			Assimp::Importer importer;
			const unsigned int flags =
				aiProcess_Triangulate |
				aiProcess_JoinIdenticalVertices |
				aiProcess_GenSmoothNormals |
				aiProcess_ImproveCacheLocality |
				aiProcess_SortByPType;

			const aiScene* scene = importer.ReadFile(path.string(), flags);
			if (!scene || !scene->mRootNode || scene->mNumMeshes == 0)
				return false;

			std::vector<glm::vec3> triPositions;
			triPositions.reserve(4096);

			for (unsigned int mi = 0; mi < scene->mNumMeshes; ++mi)
			{
				const aiMesh* mesh = scene->mMeshes[mi];
				if (!mesh || !mesh->HasPositions())
					continue;

				for (unsigned int fi = 0; fi < mesh->mNumFaces; ++fi)
				{
					const aiFace& face = mesh->mFaces[fi];
					if (face.mNumIndices != 3)
						continue;

					for (unsigned int k = 0; k < 3; ++k)
					{
						const unsigned int idx = face.mIndices[k];
						if (idx >= mesh->mNumVertices)
							continue;
						const aiVector3D& v = mesh->mVertices[idx];
						triPositions.emplace_back(v.x, v.y, v.z);
					}
				}
			}

			if (triPositions.empty())
				return false;

			// Normalize to fit inside [-0.5, 0.5] cube for consistent preview scale.
			glm::vec3 mn = triPositions[0];
			glm::vec3 mx = triPositions[0];
			for (const auto& p : triPositions)
			{
				mn = glm::min(mn, p);
				mx = glm::max(mx, p);
			}
			const glm::vec3 center = 0.5f * (mn + mx);
			const glm::vec3 ext = (mx - mn);
			const float maxExtent = std::max(ext.x, std::max(ext.y, ext.z));
			const float inv = (maxExtent > 0.000001f) ? (1.0f / maxExtent) : 1.0f;

			std::vector<float> verts;
			verts.reserve(triPositions.size() * 6);
			for (auto p : triPositions)
			{
				p = (p - center) * inv; // now roughly within [-0.5, 0.5]
				verts.push_back(p.x);
				verts.push_back(p.y);
				verts.push_back(p.z);
				// vertex color (white) - multiplied by u_Tint in shader
				verts.push_back(1.0f);
				verts.push_back(1.0f);
				verts.push_back(1.0f);
			}

			m_vertexCount = static_cast<GLsizei>(triPositions.size());

			if (!m_vao) glGenVertexArrays(1, &m_vao);
			if (!m_vbo) glGenBuffers(1, &m_vbo);

			glBindVertexArray(m_vao);
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
			glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(verts.size() * sizeof(float)), verts.data(), GL_STATIC_DRAW);

			// position
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			// color
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			return true;
		}

		void Draw() const
		{
			glBindVertexArray(m_vao);
			glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
		}

		GLuint m_vao = 0;
		GLuint m_vbo = 0;
		GLsizei m_vertexCount = 0;
	};
#endif

	static GLFWwindow* CreateHiddenSharedContextWindow(GLFWwindow* shareWith)
	{
		if (!shareWith)
			return nullptr;

		// Mirror the main window's GL config and create a hidden window sharing objects.
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		GLFWwindow* w = glfwCreateWindow(1, 1, "ArkRenderThreadContext", nullptr, shareWith);
		return w;
	}

	WorldRenderThread::WorldRenderThread(GLFWwindow* shareWithWindow)
	{
		m_renderWindow = CreateHiddenSharedContextWindow(shareWithWindow);
		if (!m_renderWindow)
			throw std::runtime_error("WorldRenderThread: failed to create shared OpenGL context window");

		m_thread = std::thread([this]() { ThreadMain(); });
	}

	WorldRenderThread::~WorldRenderThread()
	{
		RequestStop();
		if (m_thread.joinable())
			m_thread.join();

		if (m_renderWindow)
		{
			// Must be destroyed before glfwTerminate() (owned by ArkWindow).
			glfwDestroyWindow(m_renderWindow);
			m_renderWindow = nullptr;
		}
	}

	void WorldRenderThread::RequestStop()
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_stop = true;
			m_hasInput = true;
		}
		m_cv.notify_one();
	}

	void WorldRenderThread::Submit(const WorldRenderInput& input)
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_input = input;
			m_hasInput = true;
		}
		m_cv.notify_one();
	}

	void WorldRenderThread::ThreadMain()
	{
		glfwMakeContextCurrent(m_renderWindow);
		glfwSwapInterval(0);

		// GLAD is process-global. It's already loaded on the main thread after a context
		// is current, but calling it again here is harmless and makes this thread robust.
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			Logging::Error() << "WorldRenderThread: Failed to initialize GLAD on render thread.\n";
			return;
		}

		glEnable(GL_DEPTH_TEST);

		// Dedicated render-thread resources (avoid relying on objects created on the main context).
		Shader viewportShader;
		if (!viewportShader.LoadFromFiles(
			"ArkEngine/Resources/Shaders/vertex.glsl",
			"ArkEngine/Resources/Shaders/fragment.glsl"))
		{
			Logging::Error() << "WorldRenderThread: Failed to load viewport shader.\n";
			return;
		}

		CubeMesh cubeMesh;
		struct CachedMesh
		{
			std::unique_ptr<ObjGpuMesh> mesh;
#if ARK_HAS_ASSIMP
			std::unique_ptr<AssimpGpuMesh> assimpMesh;
#endif
			bool failed = false;
		};
		std::unordered_map<std::string, CachedMesh> meshCache;

		ArkCamera camera(
			glm::vec3(0.0f, 0.0f, 3.0f),
			45.0f,
			1280.0f / 720.0f,
			0.1f,
			100.0f);

		Framebuffer viewportFramebuffer;

		while (true)
		{
			WorldRenderInput input{};
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_cv.wait(lock, [this]() { return m_hasInput; });
				m_hasInput = false;
				if (m_stop)
					break;
				input = m_input;
			}

			const uint32_t w = (input.width < 1) ? 1u : input.width;
			const uint32_t h = (input.height < 1) ? 1u : input.height;

			// (Re)allocate FBO as needed.
			if (viewportFramebuffer.GetColorTextureId() == 0)
			{
				if (!viewportFramebuffer.Create(w, h))
				{
					Logging::Error() << "WorldRenderThread: Failed to create viewport framebuffer.\n";
					continue;
				}
			}
			else
			{
				if (!viewportFramebuffer.Resize(w, h))
				{
					Logging::Error() << "WorldRenderThread: Failed to resize viewport framebuffer.\n";
					continue;
				}
			}

			viewportFramebuffer.Bind();
			glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
			glEnable(GL_DEPTH_TEST);
			glClearColor(0.08f, 0.09f, 0.10f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			camera.SetAspect(static_cast<float>(w) / static_cast<float>(h));
			camera.SetPosition(input.camera.position);
			camera.SetRotation(input.camera.pitchYawDeg.x, input.camera.pitchYawDeg.y);
			camera.SetFOV(input.camera.fovDeg);
			camera.SetClipPlanes(input.camera.nearPlane, input.camera.farPlane);
			viewportShader.Bind();
			for (const RenderInstance& inst : input.instances)
			{
				const glm::mat4 mvp = camera.GetViewProjection() * inst.model;
				viewportShader.SetMat4("uMVP", mvp);
				viewportShader.SetVec3("u_Tint", inst.tint);
				if (inst.meshPath.empty())
				{
					cubeMesh.Draw();
					continue;
				}

				const std::string resolvedPath = AssetManager::Instance().ResolveAssetPath(inst.meshPath);

				// Basic mesh preview: supports .obj files via a tiny built-in parser.
				// Other formats fall back to cube unless Assimp is available in this build.
				auto& entry = meshCache[resolvedPath];
				if (!entry.mesh && !entry.failed)
				{
					const std::filesystem::path p(resolvedPath);
					const std::string ext = p.has_extension() ? p.extension().string() : std::string();
					std::string extLower = ext;
					for (char& c : extLower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

					if (extLower == ".obj")
					{
						entry.mesh = std::make_unique<ObjGpuMesh>();
						if (!entry.mesh->LoadFromObj(p))
						{
							entry.mesh.reset();
							entry.failed = true;
							Logging::Warning() << "WorldRenderThread: Failed to load OBJ mesh '" << inst.meshPath << "'. Falling back to cube.\n";
						}
					}
					else
					{
#if ARK_HAS_ASSIMP
						entry.assimpMesh = std::make_unique<AssimpGpuMesh>();
						if (!entry.assimpMesh->LoadFromFile(p))
						{
							entry.assimpMesh.reset();
							entry.failed = true;
							Logging::Warning() << "WorldRenderThread: Assimp failed to load mesh '" << inst.meshPath << "'. Falling back to cube.\n";
						}
#else
						entry.failed = true;
						Logging::Warning() << "WorldRenderThread: Mesh format not supported without Assimp: '" << inst.meshPath << "'. Falling back to cube.\n";
#endif
					}
				}

				if (entry.mesh)
					entry.mesh->Draw();
#if ARK_HAS_ASSIMP
				else if (entry.assimpMesh)
					entry.assimpMesh->Draw();
#endif
				else
					cubeMesh.Draw();
			}

			Framebuffer::Unbind();

			// Ensure the texture is fully rendered before the main thread uses it.
			// (Simple correctness-first sync; can be upgraded to GL sync objects later.)
			glFinish();

			m_latestTextureId.store(viewportFramebuffer.GetColorTextureId(), std::memory_order_relaxed);
		}

		glfwMakeContextCurrent(nullptr);
	}
}