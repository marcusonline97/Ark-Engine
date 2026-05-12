#pragma once

#include <memory>
#include <unordered_map>
#include <string>

namespace Engine
{
	class ShaderProgram;

	class Material
	{
	public:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------

		//-------------------------------------------
		// Functions
		//-------------------------------------------
		void SetShaderProgram(const std::shared_ptr<ShaderProgram>& shaderProgram);

		void SetParam(const std::string& name, float value);
		void SetParam(const std::string& name, float v0, float v1);
		void Bind();

	private:

		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------
		std::shared_ptr<ShaderProgram> m_shaderProgram;

		std::unordered_map<std::string, float> m_floatParams;
		std::unordered_map<std::string, std::pair<float, float>> m_float2Params;

		//-------------------------------------------
		// Functions
		//-------------------------------------------
		



	};
}