#include <glad/glad.h>

#include "Utility/Util.h"

class IOBuffer
{
public:

    IOBuffer();

    ~IOBuffer();

    bool Init(uint WindowWidth, uint WindowHeight, bool WidthDepth, GLenum InternalType);

    void BindForWriting();

    void BindForReading(GLenum TextureUnit);

private:

    GLuint m_fbo;
    GLuint m_texture;
    GLuint m_depth;
    GLenum m_internalType;
};