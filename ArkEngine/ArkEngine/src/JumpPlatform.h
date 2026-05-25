#pragma once

#include <Core/EngineInclude.h>

class JumpPlatform : public Engine::GameObject, public Engine::IContactListener
{
    GAMEOBJECT(JumpPlatform)
public:
    void Init() override;
    void OnContact(Engine::CollisionObject* obj, const glm::vec3& pos, const glm::vec3& norm) override;
};