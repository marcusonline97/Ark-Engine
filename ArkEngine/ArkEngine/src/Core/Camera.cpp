#include "Camera.h"
#include "../Input/keycodes.h"
#include "../BackEnd/BackEnd.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

Camera::Camera() 
    : _position(0.0f, 0.0f, 0.0f),
      _front(0.0f, 0.0f, -1.0f),
      _worldUp(0.0f, 1.0f, 0.0f),
      _yaw(-90.0f),
      _pitch(0.0f),
      _movementSpeed(5.0f),
      _mouseSensitivity(0.1f),
      _fov(45.0f),
      _aspectRatio(16.0f / 9.0f),
      _nearPlane(0.1f),
      _farPlane(1000.0f),
      _updateView(true),
      _updateProjection(true) {
    UpdateCameraVectors();
}

void Camera::Initialize(glm::vec3 position, float yaw, float pitch) {
    _position = position;
    _yaw = yaw;
    _pitch = pitch;
    _updateView = true;
    UpdateCameraVectors();
}

void Camera::Update(float deltaTime) {
    // Process keyboard movement
    float velocity = _movementSpeed * deltaTime;
    
    if (_keys[FORWARD]) {
        ProcessMovement(FORWARD, deltaTime);
    }
    if (_keys[BACKWARD]) {
        ProcessMovement(BACKWARD, deltaTime);
    }
    if (_keys[LEFT]) {
        ProcessMovement(LEFT, deltaTime);
    }
    if (_keys[RIGHT]) {
        ProcessMovement(RIGHT, deltaTime);
    }
    if (_keys[UP]) {
        ProcessMovement(UP, deltaTime);
    }
    if (_keys[DOWN]) {
        ProcessMovement(DOWN, deltaTime);
    }
}

void Camera::ProcessKeyboard(int key, bool isPressed) {
    GLFWwindow* window = BackEnd::GetWindowPointer();
    if (window == nullptr) return;
    
    // WASD movement
    if (key == GLFW_KEY_W || key == ARK_KEY_W) {
        _keys[FORWARD] = isPressed;
    }
    if (key == GLFW_KEY_S || key == ARK_KEY_S) {
        _keys[BACKWARD] = isPressed;
    }
    if (key == GLFW_KEY_A || key == ARK_KEY_A) {
        _keys[LEFT] = isPressed;
    }
    if (key == GLFW_KEY_D || key == ARK_KEY_D) {
        _keys[RIGHT] = isPressed;
    }
    if (key == GLFW_KEY_SPACE || key == ARK_KEY_SPACE) {
        _keys[UP] = isPressed;
    }
    if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL || key == ARK_KEY_LEFT_CONTROL) {
        _keys[DOWN] = isPressed;
    }
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= _mouseSensitivity;
    yoffset *= _mouseSensitivity;
    
    _yaw += xoffset;
    _pitch += yoffset;
    
    // Constrain pitch
    if (constrainPitch) {
        if (_pitch > 89.0f) {
            _pitch = 89.0f;
        }
        if (_pitch < -89.0f) {
            _pitch = -89.0f;
        }
    }
    
    _updateView = true;
    UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
    _fov -= yoffset;
    if (_fov < 1.0f) {
        _fov = 1.0f;
    }
    if (_fov > 120.0f) {
        _fov = 120.0f;
    }
    _updateProjection = true;
}

void Camera::ProcessMovement(Movement direction, float deltaTime) {
    float velocity = _movementSpeed * deltaTime;
    
    switch (direction) {
        case FORWARD:
            _position += _front * velocity;
            break;
        case BACKWARD:
            _position -= _front * velocity;
            break;
        case LEFT:
            _position -= _right * velocity;
            break;
        case RIGHT:
            _position += _right * velocity;
            break;
        case UP:
            _position += _worldUp * velocity;
            break;
        case DOWN:
            _position -= _worldUp * velocity;
            break;
    }
    _updateView = true;
}

glm::mat4 Camera::GetViewMatrix() const {
    if (_updateView) {
        _viewMatrix = glm::lookAt(_position, _position + _front, _up);
        _updateView = false;
    }
    return _viewMatrix;
}

glm::mat4 Camera::GetProjectionMatrix() const {
    if (_updateProjection) {
        _projectionMatrix = glm::perspective(glm::radians(_fov), _aspectRatio, _nearPlane, _farPlane);
        _updateProjection = false;
    }
    return _projectionMatrix;
}

glm::mat4 Camera::GetViewProjectionMatrix() const {
    return GetProjectionMatrix() * GetViewMatrix();
}

void Camera::UpdateCameraVectors() {
    // Calculate the new front vector
    glm::vec3 front;
    front.x = std::cos(glm::radians(_yaw)) * std::cos(glm::radians(_pitch));
    front.y = std::sin(glm::radians(_pitch));
    front.z = std::sin(glm::radians(_yaw)) * std::cos(glm::radians(_pitch));
    _front = glm::normalize(front);
    
    // Recalculate right and up vectors
    _right = glm::normalize(glm::cross(_front, _worldUp));
    _up = glm::normalize(glm::cross(_right, _front));
    
    _updateView = true;
}
