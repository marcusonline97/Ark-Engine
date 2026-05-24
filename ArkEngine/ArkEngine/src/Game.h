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
	void RegisterTypes() override;
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
	std::shared_ptr<Engine::Scene> m_scene;
};