#pragma once
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

class ArkCamera
{
public:
    ArkCamera(glm::vec3 position, float fov, float aspect, float nearPlane, float farPlane);

    void SetPosition(const glm::vec3& pos);
    void SetRotation(float pitch, float yaw);

    void Move(const glm::vec3& delta);
    void Rotate(float pitchDelta, float yawDelta);

    void SetAspect(float aspect);
    void SetFOV(float fovDeg);
    void SetClipPlanes(float nearPlane, float farPlane);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
    glm::mat4 GetViewProjection() const;

private:
    glm::vec3 m_Position;

    float m_Pitch;
    float m_Yaw;
    float m_FOV;
    float m_Aspect;
    float m_Near;
    float m_Far;

    mutable bool m_viewDirty = true;
    mutable bool m_projDirty = true;
    mutable glm::mat4 m_viewMatrix{ 1.0f };
    mutable glm::mat4 m_projectionMatrix{ 1.0f };
};
