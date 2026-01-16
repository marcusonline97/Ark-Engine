#pragma once
#include <list>
#include <Glad/glad.h>

class Technique
{
public:

    Technique();

    virtual ~Technique();

    virtual bool Init();

    void Enable();

    GLuint GetProgram() const { return m_shaderProg; }

protected:

    bool AddShader(GLenum ShaderType, const char* pFilename);

    bool Finalize();

    GLint GetUniformLocation(const char* pUniformName);

    GLuint m_shaderProg = 0;

private:

    void PrintUniformList();

    typedef std::list<GLuint> ShaderObjList;
    ShaderObjList m_shaderObjList;
};

#ifdef FAIL_ON_MISSING_LOC                  
#define GET_UNIFORM_AND_CHECK(loc, name)    \
    loc = GetUniformLocation(name);         \
    if (loc == INVALID_UNIFORM_LOCATION)    \
        return false;                       
#else
#define GET_UNIFORM_AND_CHECK(loc, name)    \
    loc = GetUniformLocation(name);         
#endif