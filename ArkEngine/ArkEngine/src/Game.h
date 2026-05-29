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
	std::shared_ptr<Engine::Scene> m_scene;
	Engine::GameObject* m_3DRoot = nullptr;

	//-------------------------------------------
	// Functions
	//-------------------------------------------
};