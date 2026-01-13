#include "Shader.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "AssetManager.h"

namespace
{
    // Single-threaded render loop: avoid redundant glUseProgram calls.
    static GLuint g_BoundProgram = 0;
}



Shader::Shader(Shader&& other) noexcept
{
    *this = std::move(other);
}

Shader::~Shader()
{
    if (m_program != 0)
    {
        if (g_BoundProgram == m_program)
            g_BoundProgram = 0;
        glDeleteProgram(m_program);
        m_program = 0;
    }
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this == &other) return *this;

    if (m_program != 0)
    {
        if (g_BoundProgram == m_program)
            g_BoundProgram = 0;
        glDeleteProgram(m_program);
    }

    m_program = other.m_program;
    m_uniformLocationCache = std::move(other.m_uniformLocationCache);

    other.m_program = 0;
    other.m_uniformLocationCache.clear();
    return *this;
}
static std::string ReadFile(const std::string& path) {
    std::ifstream file(path, std::ios::in);
    if (!file) throw std::runtime_error("Failed to open file: " + path);
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

bool Shader::LoadFromFiles(const std::string& vsPath, const std::string& fsPath) {
    const std::string vsResolved = AssetManager::Instance().ResolveAssetPath(vsPath);
    const std::string fsResolved = AssetManager::Instance().ResolveAssetPath(fsPath);

    std::string vsSrc = ReadFile(vsResolved);
    std::string fsSrc = ReadFile(fsResolved);

    const char* vsCode = vsSrc.c_str();
    const char* fsCode = fsSrc.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsCode, nullptr);
    glCompileShader(vs);
    GLint ok = GL_FALSE;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048]; GLsizei len = 0;
        glGetShaderInfoLog(vs, sizeof(log), &len, log);
        glDeleteShader(vs);
        throw std::runtime_error(std::string("Vertex shader compile error: ") + log);
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsCode, nullptr);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048]; GLsizei len = 0;
        glGetShaderInfoLog(fs, sizeof(log), &len, log);
        glDeleteShader(vs);
        glDeleteShader(fs);
        throw std::runtime_error(std::string("Fragment shader compile error: ") + log);
    }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);

    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048]; GLsizei len = 0;
        glGetProgramInfoLog(prog, sizeof(log), &len, log);
        glDeleteProgram(prog);
        throw std::runtime_error(std::string("Program link error: ") + log);
    }

    if (m_program != 0)
    {
        if (g_BoundProgram == m_program)
            g_BoundProgram = 0;
        glDeleteProgram(m_program);
    }

    m_program = prog;
    m_uniformLocationCache.clear();

    return true;
}

void Shader::Bind() const
{
    if (g_BoundProgram != m_program)
    {
        glUseProgram(m_program);
        g_BoundProgram = m_program;
    }
}

int Shader::GetLocation(const char* name) const
{
    if (!name || m_program == 0)
        return -1;

    const auto it = m_uniformLocationCache.find(name);
    if (it != m_uniformLocationCache.end())
        return it->second;

    const int loc = glGetUniformLocation(m_program, name);
    m_uniformLocationCache.emplace(name, loc);
    return loc;
}

void Shader::SetInt(const char* name, int v) const { glUniform1i(GetLocation(name), v); }
void Shader::SetFloat(const char* name, float v) const { glUniform1f(GetLocation(name), v); }
void Shader::SetVec3(const char* name, const glm::vec3& v) const { glUniform3fv(GetLocation(name), 1, &v.x); }
void Shader::SetMat4(const char* name, const glm::mat4& m) const { glUniformMatrix4fv(GetLocation(name), 1, GL_FALSE, &m[0][0]); }