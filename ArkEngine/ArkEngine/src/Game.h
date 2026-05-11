#pragma once
#include "Core/EngineInclude.h"

class Game : public Engine::Application
{

public:
	//-------------------------------------------
	// Properties & Variables
	//-------------------------------------------


	//-------------------------------------------
	// Functions
	//-------------------------------------------

	bool Init() override;

	void Update(float deltaTime) override;

	void Destroy() override;
};