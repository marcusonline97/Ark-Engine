#pragma once

#include "Core/EngineInclude.h"

class ShootableCube : public Engine::GameObject, public Engine::IContactListener
{
    GAMEOBJECT(ShootableCube)

public:
    ~ShootableCube() override;

    void Init() override;
    void OnContact(Engine::CollisionObject* obj, const glm::vec3& pos, const glm::vec3& norm) override;

private:
    Engine::RigidBody* m_rigidBody = nullptr;
    bool m_wasShot = false;
};
