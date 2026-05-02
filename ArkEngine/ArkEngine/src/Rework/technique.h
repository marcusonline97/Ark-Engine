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

// DEPRECATED
#ifdef FAIL_ON_MISSING_LOC                  
#define GET_UNIFORM_AND_CHECK(loc, name)    \
    loc = GetUniformLocation(name);         \
    if (loc == INVALID_UNIFORM_LOCATION)    \
        return false;                       
#else
#define GET_UNIFORM_AND_CHECK(loc, name)    \
    loc = GetUniformLocation(name);         
#endif

#define DEF_LOC_OLD(name) GLuint name = INVALID_UNIFORM_LOCATION

// NEW
#ifdef FAIL_ON_MISSING_LOC                  
#define GET_UNIFORM(name)    \
    m_##name##Loc = GetUniformLocation(#name); if (m_##name##Loc == INVALID_UNIFORM_LOCATION) return false
#else
#define GET_UNIFORM(name)    \
    m_##name##Loc = GetUniformLocation(#name)         
#endif

#define DEF_LOC(name) GLuint m_##name##Loc = -1