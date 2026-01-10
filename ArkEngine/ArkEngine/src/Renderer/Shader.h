#pragma once
#include "../Common.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <unordered_map>

class Shader {
public:
    Shader();
    ~Shader();
    
    // Load shader from vertex and fragment shader files
    bool LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    
    // Load shader from vertex and fragment shader source strings
    bool LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    
    // Use/activate the shader
    void Use() const;
    
    // Uniform setting functions
    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    void SetVec2(const std::string& name, float x, float y) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetVec3(const std::string& name, float x, float y, float z) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetVec4(const std::string& name, float x, float y, float z, float w) const;
    void SetMat2(const std::string& name, const glm::mat2& mat) const;
    void SetMat3(const std::string& name, const glm::mat3& mat) const;
    void SetMat4(const std::string& name, const glm::mat4& mat) const;
    
    // Get shader program ID
    GLuint GetID() const { return _ID; }
    
    // Check if shader is valid
    bool IsValid() const { return _ID != 0; }
    
    // Reload shader from files (for hot-reloading)
    bool Reload();
    
private:
    GLuint _ID;
    std::string _vertexPath;
    std::string _fragmentPath;
    mutable std::unordered_map<std::string, GLint> _uniformLocationCache;
    
    // Helper functions
    std::string ReadFile(const std::string& filepath);
    GLuint CompileShader(const std::string& source, GLenum shaderType);
    GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
    GLint GetUniformLocation(const std::string& name) const;
    void CheckCompileErrors(GLuint shader, const std::string& type);
};
