#include "Shader.h"
#include "../Common.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader() : _ID(0) {
}

Shader::~Shader() {
    if (_ID != 0) {
        glDeleteProgram(_ID);
        _ID = 0;
    }
}

std::string Shader::ReadFile(const std::string& filepath) {
    std::string code;
    std::ifstream file;
    
    // Try multiple possible paths
    std::vector<std::string> possiblePaths = {
        filepath,  // Try as-is first
        "Resources/Shaders/" + filepath,
        "res/shaders/OpenGL/" + filepath,
        "res/shaders/" + filepath,
    };
    
    bool fileOpened = false;
    std::string fullPath;
    
    for (const auto& path : possiblePaths) {
        file.open(path);
        if (file.is_open()) {
            fullPath = path;
            fileOpened = true;
            break;
        }
        file.close();
    }
    
    if (!fileOpened) {
        std::cout << "ERROR::SHADER::FILE_NOT_FOUND: " << filepath << "\n";
        std::cout << "  Tried paths:\n";
        for (const auto& path : possiblePaths) {
            std::cout << "    - " << path << "\n";
        }
        return "";
    }
    
    // Ensure ifstream objects can throw exceptions
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    try {
        std::stringstream stream;
        // Read file's buffer contents into stream
        stream << file.rdbuf();
        // Close file handler
        file.close();
        // Convert stream into string
        code = stream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << fullPath << "\n";
        std::cout << "Exception: " << e.what() << "\n";
        return "";
    }
    
    return code;
}

void Shader::CheckCompileErrors(GLuint shader, const std::string& type) {
    GLint success;
    GLchar infoLog[1024];
    
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n";
            std::cout << infoLog << "\n";
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n";
            std::cout << infoLog << "\n";
        }
    }
}

GLuint Shader::CompileShader(const std::string& source, GLenum shaderType) {
    const char* shaderCode = source.c_str();
    
    // Create shader
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderCode, nullptr);
    glCompileShader(shader);
    
    // Check for compilation errors
    std::string shaderTypeStr = (shaderType == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
    CheckCompileErrors(shader, shaderTypeStr);
    
    return shader;
}

GLuint Shader::LinkProgram(GLuint vertexShader, GLuint fragmentShader) {
    // Create shader program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // Check for linking errors
    CheckCompileErrors(program, "PROGRAM");
    
    // Delete shaders after linking (they're no longer needed)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

bool Shader::LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    _vertexPath = vertexPath;
    _fragmentPath = fragmentPath;
    
    // Read shader source from files
    std::string vertexCode = ReadFile(vertexPath);
    std::string fragmentCode = ReadFile(fragmentPath);
    
    if (vertexCode.empty() || fragmentCode.empty()) {
        std::cout << "ERROR::SHADER::Failed to read shader files\n";
        return false;
    }
    
    return LoadFromSource(vertexCode, fragmentCode);
}

bool Shader::LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    // Delete existing program if it exists
    if (_ID != 0) {
        glDeleteProgram(_ID);
        _ID = 0;
    }
    
    // Clear uniform cache
    _uniformLocationCache.clear();
    
    // Compile shaders
    GLuint vertexShader = CompileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = CompileShader(fragmentSource, GL_FRAGMENT_SHADER);
    
    // Check if compilation was successful
    GLint vertexSuccess, fragmentSuccess;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexSuccess);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentSuccess);
    
    if (!vertexSuccess || !fragmentSuccess) {
        // Clean up shaders
        if (vertexShader != 0) glDeleteShader(vertexShader);
        if (fragmentShader != 0) glDeleteShader(fragmentShader);
        std::cout << "ERROR::SHADER::Shader compilation failed\n";
        return false;
    }
    
    // Link program
    _ID = LinkProgram(vertexShader, fragmentShader);
    
    // Check if linking was successful
    GLint linkSuccess;
    glGetProgramiv(_ID, GL_LINK_STATUS, &linkSuccess);
    
    if (!linkSuccess) {
        if (_ID != 0) {
            glDeleteProgram(_ID);
            _ID = 0;
        }
        std::cout << "ERROR::SHADER::Shader program linking failed\n";
        return false;
    }
    
    return true;
}

void Shader::Use() const {
    if (_ID != 0) {
        glUseProgram(_ID);
    }
}

GLint Shader::GetUniformLocation(const std::string& name) const {
    // Check cache first
    auto it = _uniformLocationCache.find(name);
    if (it != _uniformLocationCache.end()) {
        return it->second;
    }
    
    // Query OpenGL
    GLint location = glGetUniformLocation(_ID, name.c_str());
    
    // Cache result (even if -1, to avoid repeated queries)
    _uniformLocationCache[name] = location;
    
    if (location == -1) {
        std::cout << "WARNING::SHADER::Uniform '" << name << "' not found (or optimized away)\n";
    }
    
    return location;
}

void Shader::SetBool(const std::string& name, bool value) const {
    glUniform1i(GetUniformLocation(name), static_cast<int>(value));
}

void Shader::SetInt(const std::string& name, int value) const {
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetFloat(const std::string& name, float value) const {
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetVec2(const std::string& name, float x, float y) const {
    glUniform2f(GetUniformLocation(name), x, y);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetVec3(const std::string& name, float x, float y, float z) const {
    glUniform3f(GetUniformLocation(name), x, y, z);
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetVec4(const std::string& name, float x, float y, float z, float w) const {
    glUniform4f(GetUniformLocation(name), x, y, z, w);
}

void Shader::SetMat2(const std::string& name, const glm::mat2& mat) const {
    glUniformMatrix2fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::SetMat3(const std::string& name, const glm::mat3& mat) const {
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::SetMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}

bool Shader::Reload() {
    if (!_vertexPath.empty() && !_fragmentPath.empty()) {
        std::string vertexPath = _vertexPath;
        std::string fragmentPath = _fragmentPath;
        return LoadFromFiles(vertexPath, fragmentPath);
    }
    return false;
}
