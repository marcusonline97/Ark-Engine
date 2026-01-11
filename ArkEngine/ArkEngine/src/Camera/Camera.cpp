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
}

void ArkCamera::SetPosition(const glm::vec3& pos)
{
    m_Position = pos;
}

void ArkCamera::SetRotation(float pitch, float yaw)
{
    m_Pitch = pitch;
    m_Yaw = yaw;
}

void ArkCamera::Move(const glm::vec3& delta)
{
    m_Position += delta;
}

void ArkCamera::Rotate(float pitchDelta, float yawDelta)
{
    m_Pitch += pitchDelta;
    m_Yaw += yawDelta;
}

glm::mat4 ArkCamera::GetViewMatrix() const
{
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

    return glm::lookAt(m_Position, m_Position + glm::normalize(front), glm::vec3(0, 1, 0));
}

glm::mat4 ArkCamera::GetProjectionMatrix() const
{
    return glm::perspective(glm::radians(m_FOV), m_Aspect, m_Near, m_Far);
}

glm::mat4 ArkCamera::GetViewProjection() const
{
    return GetProjectionMatrix() * GetViewMatrix();
}
