#pragma once
#include "Core/EngineInclude.h"
#include <memory>
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

private:
	//-------------------------------------------
	// Properties & Variables
	//-------------------------------------------
	
	//-------------------------------------------
	// Functions
	//-------------------------------------------
	Engine::Scene* m_scene;
};