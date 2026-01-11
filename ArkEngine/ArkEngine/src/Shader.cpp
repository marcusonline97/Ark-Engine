#include "Shader.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
    std::string vertexSrc = LoadFile(vertexPath);
    std::string fragmentSrc = LoadFile(fragmentPath);

    if (vertexSrc.empty())
    {
        std::cerr << "Failed to load vertex shader file: " << vertexPath << std::endl;
        throw std::runtime_error("Vertex shader source is empty");
    }
    if (fragmentSrc.empty())
    {
        std::cerr << "Failed to load fragment shader file: " << fragmentPath << std::endl;
        throw std::runtime_error("Fragment shader source is empty");
    }

    std::cout << "Vertex shader source:\n" << vertexSrc << "\n";
    std::cout << "Fragment shader source:\n" << fragmentSrc << "\n";

    m_RendererID = CreateProgram(vertexSrc, fragmentSrc);
}

Shader::~Shader()
{
    glDeleteProgram(m_RendererID);
}

void Shader::Bind() const
{
    glUseProgram(m_RendererID);
}

void Shader::Unbind() const
{
    glUseProgram(0);
}

std::string Shader::LoadFile(const std::string& path)
{
    std::ifstream file(path, std::ios::in);
    if (!file.is_open())
    {
        std::cerr << "Shader::LoadFile failed to open: " << path << std::endl;
        return {};
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetShaderInfoLog(id, 1024, nullptr, infoLog);
        std::cerr << "Shader compilation failed:\n" << infoLog << std::endl;
    }

    return id;
}

unsigned int Shader::CreateProgram(const std::string& vertexSrc, const std::string& fragmentSrc)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        std::cerr << "Shader linking failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void Shader::SetMat4(const std::string& name, const glm::mat4& matrix)
{
    int loc = glGetUniformLocation(m_RendererID, name.c_str());
    if (loc != -1)
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::SetFloat(const std::string& name, float value)
{
    int loc = glGetUniformLocation(m_RendererID, name.c_str());
    if (loc != -1)
        glUniform1f(loc, value);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& vec)
{
    int loc = glGetUniformLocation(m_RendererID, name.c_str());
    if (loc != -1)
        glUniform3fv(loc, 1, glm::value_ptr(vec));
}
