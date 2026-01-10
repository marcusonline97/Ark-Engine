#include "BackEnd.h"
#include <iostream>
#include <string>
#include "../API/OpenGL/GL_backEnd.h"
#include "../API/Vulkan/VK_backEnd.h"

namespace BackEnd {

    API _api = API::UNDEFINED;
    GLFWwindow* _window = NULL;
    WindowedMode _windowedMode = WindowedMode::WINDOWED;
    GLFWmonitor* _monitor;
    const GLFWvidmode* _mode;
    bool _forceCloseWindow = false;
    bool _windowHasFocus = true;
    int _windowedWidth = 0;
    int _windowedHeight = 0;
    int _fullscreenWidth = 0;
    int _fullscreenHeight = 0;
    int _currentWindowWidth = 0;
    int _currentWindowHeight = 0;
    int _presentTargetWidth = 0;
    int _presentTargetHeight = 0;

    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void window_focus_callback(GLFWwindow* window, int focused);


    ////////////////////
    //                //
    //      Core      //

    void Init(API api) {

        _api = api;
        bool openGLSucceeded = false;

        int width = 1280;
        int height = 720;

        // Initialize GLFW
        if (!glfwInit()) {
            std::cout << "ERROR: Failed to initialize GLFW\n";
            return;
        }

        // Set error callback for GLFW errors
        glfwSetErrorCallback([](int error, const char* description) { 
            std::cout << "GLFW Error (" << std::to_string(error) << "): " << description << "\n"; 
        });

        // Try OpenGL first if requested
        if (GetAPI() == API::OPENGL) {
            // Set OpenGL context hints
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
            
            // Get monitor information
            _monitor = glfwGetPrimaryMonitor();
            if (_monitor == nullptr) {
                std::cout << "WARNING: Failed to get primary monitor, using defaults\n";
                _fullscreenWidth = 1920;
                _fullscreenHeight = 1080;
            }
            else {
                _mode = glfwGetVideoMode(_monitor);
                if (_mode != nullptr) {
                    glfwWindowHint(GLFW_RED_BITS, _mode->redBits);
                    glfwWindowHint(GLFW_GREEN_BITS, _mode->greenBits);
                    glfwWindowHint(GLFW_BLUE_BITS, _mode->blueBits);
                    glfwWindowHint(GLFW_REFRESH_RATE, _mode->refreshRate);
                    _fullscreenWidth = _mode->width;
                    _fullscreenHeight = _mode->height;
                }
                else {
                    std::cout << "WARNING: Failed to get video mode, using defaults\n";
                    _fullscreenWidth = 1920;
                    _fullscreenHeight = 1080;
                }
            }
            
            _windowedWidth = width;
            _windowedHeight = height;
            
            // Window creation hints
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
            
            // Create window
            CreateGLFWWindow(WindowedMode::WINDOWED);
            if (_window == nullptr) {
                std::cout << "ERROR: Failed to create GLFW window for OpenGL\n";
                glfwTerminate();
                return;
            }
            
            // Make OpenGL context current
            glfwMakeContextCurrent(_window);
            
            // Initialize OpenGL
            if (OpenGLBackEnd::InitMinimum()) {
                std::cout << "OpenGL initialized successfully!\n";
                
                // Enable VSync
                glfwSwapInterval(1);
                
                // Set initial viewport
                int framebufferWidth, framebufferHeight;
                glfwGetFramebufferSize(_window, &framebufferWidth, &framebufferHeight);
                glViewport(0, 0, framebufferWidth, framebufferHeight);
                
                // Enable depth testing
                glEnable(GL_DEPTH_TEST);
                
                openGLSucceeded = true;
            }
            else {
                std::cout << "OpenGL initialization failed, attempting Vulkan fallback...\n";
                glfwDestroyWindow(_window);
                _window = nullptr;
                _api = API::VULKAN;
                // Reset window hints for Vulkan
                glfwDefaultWindowHints();
            }
        }

        // If OpenGL failed or Vulkan was requested, try Vulkan
        if (!openGLSucceeded || GetAPI() == API::VULKAN) {
            std::cout << "Initializing Vulkan...\n";
            
            // Check if Vulkan is supported
            if (!glfwVulkanSupported()) {
                std::cout << "ERROR: Vulkan is not supported on this system!\n";
                if (!openGLSucceeded) {
                    std::cout << "ERROR: Both OpenGL and Vulkan initialization failed. Exiting.\n";
                    if (_window != nullptr) {
                        glfwDestroyWindow(_window);
                        _window = nullptr;
                    }
                    glfwTerminate();
                    return;
                }
                // If OpenGL worked, just use it
                _api = API::OPENGL;
            }
            else {
                // Reset window hints for Vulkan (important if coming from OpenGL failure)
                if (!openGLSucceeded) {
                    glfwDefaultWindowHints();
                }
                
                // Create Vulkan instance
                VulkanBackEnd::CreateVulkanInstance();
                // Note: CreateVulkanInstance doesn't return bool, so we proceed
                // If it fails, InitMinimum will catch it
                
                // Set Vulkan window hints
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
                glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
                glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
                glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
                
                // Get monitor information if not already set
                if (_monitor == nullptr) {
                    _monitor = glfwGetPrimaryMonitor();
                    if (_monitor != nullptr) {
                        _mode = glfwGetVideoMode(_monitor);
                        if (_mode != nullptr) {
                            _fullscreenWidth = _mode->width;
                            _fullscreenHeight = _mode->height;
                        }
                        else {
                            _fullscreenWidth = 1920;
                            _fullscreenHeight = 1080;
                        }
                    }
                    else {
                        _fullscreenWidth = 1920;
                        _fullscreenHeight = 1080;
                    }
                }
                
                _windowedWidth = width;
                _windowedHeight = height;
                
                // Create window for Vulkan (if not already created)
                if (_window == nullptr) {
                    CreateGLFWWindow(WindowedMode::WINDOWED);
                    if (_window == nullptr) {
                        std::cout << "ERROR: Failed to create GLFW window for Vulkan\n";
                        glfwTerminate();
                        return;
                    }
                }
                
                // Initialize Vulkan minimum
                // Note: InitMinimum() uses try-catch internally, but VK_CHECK() will abort on errors
                // If Vulkan init fails here, the program may abort (acceptable for Phase 1 minimal)
                VulkanBackEnd::InitMinimum();
                std::cout << "Vulkan initialized successfully (basic setup complete)\n";
            }
        }

        // Set callbacks
        if (_window != nullptr) {
            glfwSetFramebufferSizeCallback(_window, framebuffer_size_callback);
            glfwSetWindowFocusCallback(_window, window_focus_callback);
            
            // Show window
            glfwShowWindow(_window);
        }
    }

    void BeginFrame() {
        if (_window != nullptr) {
            glfwPollEvents();
        }
    }

    void EndFrame() {
        if (_window == nullptr) {
            return;
        }
        
        // OpenGL
        if (GetAPI() == API::OPENGL) {
            glfwSwapBuffers(_window);
        }
        // Vulkan
        else if (GetAPI() == API::VULKAN) {
            // Vulkan swapchain presentation will be handled by Vulkan renderer
            // For now, just a placeholder
        }
    }

    void UpdateSubSystems() {
        // Commented out for minimal implementation
        // Input::Update();
        // Audio::Update();
        // Scene::Update();
    }

    void CleanUp() {
        if (_window != nullptr) {
            glfwDestroyWindow(_window);
            _window = nullptr;
        }
        glfwTerminate();
    }

    ///////////////////
    //               //
    //      API      //

    void SetAPI(API api) {
        _api = api;
    }

    const API GetAPI() {
        return _api;
    }

    // Window
    GLFWwindow* GetWindowPointer() {
        return _window;
    }

    void SetWindowPointer(GLFWwindow* window) {
        _window = window;
    }

    void CreateGLFWWindow(const WindowedMode& windowedMode) {
        // Destroy existing window if any
        if (_window != nullptr) {
            glfwDestroyWindow(_window);
            _window = nullptr;
        }
        
        if (windowedMode == WindowedMode::WINDOWED) {
            _currentWindowWidth = _windowedWidth;
            _currentWindowHeight = _windowedHeight;
            _window = glfwCreateWindow(_windowedWidth, _windowedHeight, "Ark Engine", nullptr, nullptr);
            if (_window != nullptr) {
                glfwSetWindowPos(_window, 0, 0);
            }
        }
        else if (windowedMode == WindowedMode::FULLSCREEN) {
            _currentWindowWidth = _fullscreenWidth;
            _currentWindowHeight = _fullscreenHeight;
            _window = glfwCreateWindow(_fullscreenWidth, _fullscreenHeight, "Ark Engine", _monitor, nullptr);
        }
        _windowedMode = windowedMode;
    }

    void SetWindowedMode(const WindowedMode& windowedMode) {
        if (_window == nullptr) {
            return;
        }
        
        if (windowedMode == WindowedMode::WINDOWED) {
            _currentWindowWidth = _windowedWidth;
            _currentWindowHeight = _windowedHeight;
            if (_mode != nullptr) {
                glfwSetWindowMonitor(_window, nullptr, 0, 0, _windowedWidth, _windowedHeight, _mode->refreshRate);
                glfwSetWindowPos(_window, 0, 0);
            }
        }
        else if (windowedMode == WindowedMode::FULLSCREEN) {
            _currentWindowWidth = _fullscreenWidth;
            _currentWindowHeight = _fullscreenHeight;
            if (_mode != nullptr && _monitor != nullptr) {
                glfwSetWindowMonitor(_window, _monitor, 0, 0, _fullscreenWidth, _fullscreenHeight, _mode->refreshRate);
            }
        }
        _windowedMode = windowedMode;
    }

    void ToggleFullscreen() {
        if (_window == nullptr) {
            return;
        }
        
        if (_windowedMode == WindowedMode::WINDOWED) {
            SetWindowedMode(WindowedMode::FULLSCREEN);
        }
        else {
            SetWindowedMode(WindowedMode::WINDOWED);
        }
        
        // Handle API-specific resize
        if (GetAPI() == API::OPENGL) {
            // OpenGL handles resize via framebuffer callback
            // Viewport will be updated automatically
        }
        else if (GetAPI() == API::VULKAN) {
            VulkanBackEnd::HandleFrameBufferResized();
        }
    }

    void ForceCloseWindow() {
        _forceCloseWindow = true;
    }

    bool WindowHasFocus() {
        return _windowHasFocus;
    }

    bool WindowHasNotBeenForceClosed() {
        return !_forceCloseWindow;
    }

    int GetWindowedWidth() {
        return _windowedWidth;
    }

    int GetWindowedHeight() {
        return _windowedHeight;
    }

    int GetFullScreenWidth() {
        return _fullscreenWidth;
    }

    int GetFullScreenHeight() {
        return _fullscreenHeight;
    }

    int GetCurrentWindowWidth() {
        return _currentWindowWidth;
    }

    int GetCurrentWindowHeight() {
        return _currentWindowHeight;
    }

    bool WindowIsOpen() {
        if (_window == nullptr) {
            return false;
        }
        return !(glfwWindowShouldClose(_window) || _forceCloseWindow);
    }

    bool WindowIsMinimized() {
        if (_window == nullptr) {
            return true;
        }
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(_window, &width, &height);
        return (width == 0 || height == 0);
    }

    const WindowedMode& GetWindowMode() {
        return _windowedMode;
    }

    //////////////////////////////
    //                          //
    //      Render Targets      //

    void SetPresentTargetSize(int width, int height) {
        _presentTargetWidth = width;
        _presentTargetHeight = height;
        if (GetAPI() == API::OPENGL) {
            //OpenGLBackEnd::SetPresentTargetSize(width, height);
        }
        else {
            //VulkanBackEnd::SetPresentTargetSize(width, height);
        }
    }

    int GetPresentTargetWidth() {
        return _presentTargetWidth;
    }

    int GetPresentTargetHeight() {
        return _presentTargetHeight;
    }


    /////////////////////////
    //                     //
    //      Callbacks      //

    void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
        // Update viewport for OpenGL
        if (GetAPI() == API::OPENGL) {
            glViewport(0, 0, width, height);
        }
        // Handle Vulkan framebuffer resize
        else if (GetAPI() == API::VULKAN) {
            VulkanBackEnd::MarkFrameBufferAsResized();
        }
    }

    void window_focus_callback(GLFWwindow* /*window*/, int focused) {
        if (focused) {
            BackEnd::_windowHasFocus = true;
        }
        else {
            BackEnd::_windowHasFocus = false;
        }
    }
}