#pragma once
#include "../Common.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera();
    
    // Initialize camera with position and orientation
    void Initialize(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
                    float yaw = -90.0f, float pitch = 0.0f);
    
    // Update camera based on input (call this every frame)
    void Update(float deltaTime);
    
    // Input handling
    void ProcessKeyboard(int key, bool isPressed);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);
    
    // Get matrices
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
    glm::mat4 GetViewProjectionMatrix() const;
    
    // Get camera vectors
    glm::vec3 GetPosition() const { return _position; }
    glm::vec3 GetFront() const { return _front; }
    glm::vec3 GetUp() const { return _up; }
    glm::vec3 GetRight() const { return _right; }
    
    // Settings
    void SetPosition(glm::vec3 position) { _position = position; }
    void SetMovementSpeed(float speed) { _movementSpeed = speed; }
    void SetMouseSensitivity(float sensitivity) { _mouseSensitivity = sensitivity; }
    void SetFOV(float fov) { _fov = fov; _updateProjection = true; }
    void SetAspectRatio(float aspect) { _aspectRatio = aspect; _updateProjection = true; }
    void SetNearPlane(float nearPlane) { _nearPlane = nearPlane; _updateProjection = true; }
    void SetFarPlane(float farPlane) { _farPlane = farPlane; _updateProjection = true; }
    
    float GetFOV() const { return _fov; }
    float GetAspectRatio() const { return _aspectRatio; }
    float GetNearPlane() const { return _nearPlane; }
    float GetFarPlane() const { return _farPlane; }
    
    // First-person camera controls
    enum Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };
    
    void ProcessMovement(Movement direction, float deltaTime);
    
private:
    // Camera attributes
    glm::vec3 _position;
    glm::vec3 _front;
    glm::vec3 _up;
    glm::vec3 _right;
    glm::vec3 _worldUp;
    
    // Euler angles
    float _yaw;
    float _pitch;
    
    // Camera options
    float _movementSpeed;
    float _mouseSensitivity;
    float _fov;
    float _aspectRatio;
    float _nearPlane;
    float _farPlane;
    
    // Matrices
    mutable glm::mat4 _viewMatrix;
    mutable glm::mat4 _projectionMatrix;
    mutable bool _updateView;
    mutable bool _updateProjection;
    
    // Input state
    bool _keys[6] = { false, false, false, false, false, false }; // FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN
    
    // Update camera vectors based on Euler angles
    void UpdateCameraVectors();
};
