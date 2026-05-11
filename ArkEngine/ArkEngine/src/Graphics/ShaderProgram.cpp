#include "ShaderProgram.h"


namespace Engine
{
	void ShaderProgram::Bind()
	{
		glUseProgram(m_shaderProgramID);
	}

	GLint ShaderProgram::GetUniformLocation(const std::string& name)
	{
		auto it = m_uniformLocationCache.find(name);
		if (it != m_uniformLocationCache.end())
		{
			return it->second;
		}

		GLint location = glGetUniformLocation(m_shaderProgramID, name.c_str());
		m_uniformLocationCache[name] = location;

	}
	void ShaderProgram::SetUniform(const std::string& name, float value)
	{
		auto location = GetUniformLocation(name);
		glUniform1f(location, value);
	}
}
