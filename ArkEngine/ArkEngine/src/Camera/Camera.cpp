#include "Camera.h"

ArkCamera::ArkCamera(glm::vec3 position, float fov, float aspect, float nearPlane, float farPlane)
    : m_Position(position),
    m_Pitch(0.0f),
    m_Yaw(-90.0f),
    m_FOV(fov),
    m_Aspect(aspect),
    m_Near(nearPlane),
    m_Far(farPlane)
{
    m_viewDirty = true;
    m_projDirty = true;
}

void ArkCamera::SetPosition(const glm::vec3& pos)
{
    m_Position = pos;
    m_viewDirty = true;
}

void ArkCamera::SetRotation(float pitch, float yaw)
{
    m_Pitch = pitch;
    m_Yaw = yaw;
    m_viewDirty = true;
}

void ArkCamera::Move(const glm::vec3& delta)
{
    m_Position += delta;
    m_viewDirty = true;
}

void ArkCamera::Rotate(float pitchDelta, float yawDelta)
{
    m_Pitch += pitchDelta;
    m_Yaw += yawDelta;
    m_viewDirty = true;
}

void ArkCamera::SetAspect(float aspect)
{
    m_Aspect = aspect;
    m_projDirty = true;
}

void ArkCamera::SetFOV(float fovDeg)
{
    m_FOV = fovDeg;
    m_projDirty = true;
}

void ArkCamera::SetClipPlanes(float nearPlane, float farPlane)
{
    m_Near = nearPlane;
    m_Far = farPlane;
    m_projDirty = true;
}

glm::mat4 ArkCamera::GetViewMatrix() const
{
    if (m_viewDirty) {
        glm::vec3 front;
        front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        front.y = sin(glm::radians(m_Pitch));
        front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        m_viewMatrix = glm::lookAt(m_Position, m_Position + glm::normalize(front), glm::vec3(0, 1, 0));
        m_viewDirty = false;
    }

    return m_viewMatrix;
}

glm::mat4 ArkCamera::GetProjectionMatrix() const
{
    if (m_projDirty) {
        m_projectionMatrix = glm::perspective(glm::radians(m_FOV), m_Aspect, m_Near, m_Far);
        m_projDirty = false;
    }
    return m_projectionMatrix;
}

glm::mat4 ArkCamera::GetViewProjection() const
{
    return GetProjectionMatrix() * GetViewMatrix();
}
