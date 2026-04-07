#pragma once

#include <filesystem>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Ark::Rendering
{
	class ObjGpuMesh
	{
	public:
		ObjGpuMesh() = default;
		~ObjGpuMesh();

		ObjGpuMesh(const ObjGpuMesh&) = delete;
		ObjGpuMesh& operator=(const ObjGpuMesh&) = delete;

		ObjGpuMesh(ObjGpuMesh&& other) noexcept;
		ObjGpuMesh& operator=(ObjGpuMesh&& other) noexcept;

		bool LoadFromObj(const std::filesystem::path& path, bool normalizeScale = false);
		void Draw() const;

	private:
		struct ObjIndex
		{
			int v = -1;
			int vt = -1;
			int vn = -1;
		};

		static ObjIndex ParseFaceToken(const std::string& token, int vCount, int vtCount, int vnCount);
		static int FixIndex(int idx, int count);

	private:
		GLuint m_vao = 0;
		GLuint m_vbo = 0;
		GLsizei m_vertexCount = 0;
	};
}
