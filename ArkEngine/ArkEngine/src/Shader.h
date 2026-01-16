#pragma once
#include <glm/glm.hpp>

#include <string>
#include <unordered_map>

class Shader
{
public:
    Shader() = default;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;
    ~Shader();

    bool LoadFromFiles(const std::string& vsPath, const std::string& fsPath);

    void Bind() const;

    void SetInt(const char* name, int v) const;
    void SetFloat(const char* name, float v) const;
    void SetVec3(const char* name, const glm::vec3& v) const;
    void SetMat4(const char* name, const glm::mat4& m) const;

    void SetInt(std::string_view name, int v) const { SetInt(std::string(name).c_str(), v); }
    void SetFloat(std::string_view name, float v) const { SetFloat(std::string(name).c_str(), v); }
    void SetVec3(std::string_view name, const glm::vec3& v) const { SetVec3(std::string(name).c_str(), v); }
    void SetMat4(std::string_view name, const glm::mat4& m) const { SetMat4(std::string(name).c_str(), m); }

private:
    int GetLocation(const char* name) const;

private:
    unsigned int m_program = 0;
    mutable std::unordered_map<std::string, int> m_uniformLocationCache;
};