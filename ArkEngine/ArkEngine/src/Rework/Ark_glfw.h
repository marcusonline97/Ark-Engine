#include <Glad/glad.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include "Ark_glm_camera.h"

GLFWwindow* glfw_init(int major_ver, int minor_ver, int width, int height, bool is_full_screen, const char* title);
