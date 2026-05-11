#pragma once

#include <Glad/glad.h>

#include <string>
#include <unordered_map>

namespace Engine
{
	class ShaderProgram
	{

	public:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------


		//-------------------------------------------
		// Functions
		//-------------------------------------------
		void Bind();

		GLint GetUniformLocation(const std::string& name);
		
		void SetUniform(const std::string& name, float value);


	private:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------
		std::unordered_map<std::string, GLint> m_uniformLocationCache;
		GLuint m_shaderProgramID = 0;

		//-------------------------------------------
		// Functions
		//-------------------------------------------

	};
}