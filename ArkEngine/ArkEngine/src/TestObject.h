#pragma once

#include "Core/EngineInclude.h"

class TestObject : public Engine::GameObject
{

public:
	TestObject();

	void Update(float deltaTime) override;

private:

};