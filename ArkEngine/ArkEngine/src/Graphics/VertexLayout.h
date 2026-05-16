#pragma once

#include "GLAD/glad.h"

#include <vector>
#include <stdint.h>


namespace Engine
{
	struct VertexElement
	{
		GLuint index; // Attribute location in the shader
		GLuint size;  // Number of components (e.g., 3 for vec3)
		GLuint type;  // Data type (e.g., GL_FLOAT)
		uint32_t offset; // Byte offset from the start of the vertex


		static constexpr int PositionIndex = 0;
		static constexpr int ColorIndex = 1;
		static constexpr int UVIndex = 2;
	};

	struct VertexLayout
	{
		std::vector<VertexElement> elements;

		uint32_t stride = 0; // Total size of a single vertex in bytes



	};
}