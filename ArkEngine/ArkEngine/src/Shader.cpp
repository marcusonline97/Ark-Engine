#include "Shader.h"
#include <glad/glad.h>

void Shader::Bind() const { glUseProgram(m_program); }
int Shader::GetLocation(const char* name) const { return glGetUniformLocation(m_program, name); }
void Shader::SetInt(const char* name, int v) const { glUniform1i(GetLocation(name), v); }
void Shader::SetFloat(const char* name, float v) const { glUniform1f(GetLocation(name), v); }
void Shader::SetVec3(const char* name, const glm::vec3& v) const { glUniform3fv(GetLocation(name), 1, &v.x); }
void Shader::SetMat4(const char* name, const glm::mat4& m) const { glUniformMatrix4fv(GetLocation(name), 1, GL_FALSE, &m[0][0]); }