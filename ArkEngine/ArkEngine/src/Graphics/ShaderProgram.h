#pragma once

#include <Glad/glad.h>
#include <glm/mat4x4.hpp>

#include <string>
#include <unordered_map>

namespace Engine
{
	class Texture;

	class ShaderProgram
	{

	public:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------


		//-------------------------------------------
		// Functions
		//-------------------------------------------
		ShaderProgram() = delete;
		ShaderProgram(const ShaderProgram&) = delete;
		ShaderProgram& operator=(const ShaderProgram&) = delete;
		explicit ShaderProgram(GLuint shaderProgramID);
		~ShaderProgram();

		void Bind();
		GLint GetUniformLocation(const std::string& name);
		void SetUniform(const std::string& name, float value);
		void SetUniform(const std::string& name, float v0, float v1);
		void SetUniform(const std::string& name, const glm::mat4& mat);

		void SetTexture(const std::string& name, Texture* texture);
	private:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------
		std::unordered_map<std::string, GLint> m_uniformLocationCache;
		GLuint m_shaderProgramID = 0;

		int m_currentTextureUnit = 0;
		//-------------------------------------------
		// Functions
		//-------------------------------------------

	};
}