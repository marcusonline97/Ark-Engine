#pragma once
#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    void SetMat4(const std::string& name, const glm::mat4& matrix);
    void SetFloat(const std::string& name, float value);
    void SetVec3(const std::string& name, const glm::vec3& vec);

    // Expose GL program id for advanced uses (e.g., manual uniform setup)
    unsigned int GetProgramId() const { return m_RendererID; }

private:
    unsigned int m_RendererID;

    std::string LoadFile(const std::string& path);
    unsigned int CompileShader(unsigned int type, const std::string& source);
    unsigned int CreateProgram(const std::string& vertexSrc, const std::string& fragmentSrc);
};
