#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <Glad/glad.h>
#include "Ark_Types.h"


using namespace std;

bool ReadFile(const char* fileName, std::string& outFile);
char* ReadBinaryFile(const char* pFileName, int& size);

void WriteBinaryFile(const char* pFilename, const void* pData, int size);

void ArkError(const char* pFileName, uint line, const char* msg, ...);
void ArkFileError(const char* pFileName, uint line, const char* pFileError);

#define ARK_ERROR0(msg) ArkError(__FILE__, __LINE__, msg)
#define ARK_ERROR(msg, ...) ArkError(__FILE__, __LINE__, msg, __VA_ARGS__)
#define ARK_FILE_ERROR(FileError) ArkFileError(__FILE__, __LINE__, FileError);

#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ZERO_MEM_VAR(var) memset(&var, 0, sizeof(var))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define ARRAY_SIZE_IN_BYTES(a) (sizeof(a[0]) * a.size())

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifdef _WIN64
#define SNPRINTF _snprintf_s
#define VSNPRINTF vsnprintf_s
#define RANDOM rand
#define SRANDOM srand((unsigned)time(NULL))
#pragma warning (disable: 4566)
#else
#define SNPRINTF snprintf
#define VSNPRINTF vsnprintf
#define RANDOM random
#define SRANDOM srandom(getpid())
#endif

#define INVALID_UNIFORM_LOCATION 0xffffffff
#define INVALID_ARK_VALUE 0xffffffff

#define NUM_CUBE_MAP_FACES 6

#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }

long long GetCurrentTimeMillis();


#define ASSIMP_LOAD_FLAGS (aiProcess_JoinIdenticalVertices |    \
                           aiProcess_Triangulate |              \
                           aiProcess_GenSmoothNormals |         \
                           aiProcess_LimitBoneWeights |         \
                           aiProcess_SplitLargeMeshes |         \
                           aiProcess_ImproveCacheLocality |     \
                           aiProcess_RemoveRedundantMaterials | \
                           aiProcess_FindDegenerates |          \
                           aiProcess_FindInvalidData |          \
                           aiProcess_GenUVCoords |              \
                           aiProcess_CalcTangentSpace)


#define NOT_IMPLEMENTED printf("Not implemented case in %s:%d\n", __FILE__, __LINE__); exit(0);

#define GLExitIfError                                                          \
{                                                                               \
    GLenum Error = glGetError();                                                \
                                                                                \
    if (Error != GL_NO_ERROR) {                                                 \
        printf("OpenGL error in %s:%d: 0x%x\n", __FILE__, __LINE__, Error);     \
        exit(0);                                                                \
    }                                                                           \
}

#define GLCheckError() (glGetError() == GL_NO_ERROR)

void gl_check_error(const char* function, const char* file, int line);

#define CHECK_GL_ERRORS

#ifdef CHECK_GL_ERRORS
#define GCE gl_check_error(__FUNCTION__, __FILE__, __LINE__);
#else
#define GCE
#endif

void glDebugOutput(GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char* message,
    const void* userParam);

string GetDirFromFilename(const string& Filename);

#define MAX_BONES (200)

#define CLAMP(Val, Start, End) std::min(std::max((Val), (Start)), (End));

int GetGLMajorVersion();
int GetGLMinorVersion();

int IsGLVersionHigher(int MajorVer, int MinorVer);

static inline size_t AlignUpToMultiple(size_t Size, size_t Alignment)
{
    size_t ret = ((Size + Alignment - 1) / Alignment) * Alignment;

    return ret;
}