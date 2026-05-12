#pragma once

#include "Glad/glad.h"
#include "Graphics/VertexLayout.h"

namespace Engine
{
	class Mesh
	{

	public:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------

		//-------------------------------------------
		// Functions
		//-------------------------------------------
		Mesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<uint32_t>& indices);
		Mesh(const VertexLayout& layout, const std::vector<float>& vertices);


		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;

		void Bind();
		void Draw();

	private:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------
		VertexLayout m_vertexLayout;

		GLuint m_VBO = 0;
		GLuint m_EBO = 0;
		GLuint m_VAO = 0;

		size_t m_vertexCount = 0;
		size_t m_indexCount = 0;
		//-------------------------------------------
		// Functions	
		//-------------------------------------------

	};
}