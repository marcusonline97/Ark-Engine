#pragma once
#include <string>
#include <unordered_map>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class Shader {
public:
    Shader() = default;
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    bool LoadFromFiles(const std::string& vsPath, const std::string& fsPath);
    void Bind() const;
    void SetInt(const char* name, int v) const;
    void SetFloat(const char* name, float v) const;
    void SetVec3(const char* name, const glm::vec3& v) const;
    void SetMat4(const char* name, const glm::mat4& m) const;
private:
    unsigned m_program = 0;
    int GetLocation(const char* name) const;

    mutable std::unordered_map<std::string, int> m_uniformLocationCache;

};