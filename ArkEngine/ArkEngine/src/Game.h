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
	Engine::Material m_material;
	std::unique_ptr<Engine::Mesh> m_mesh;

	float m_offsetX = 0.0f;
	float m_offsetY = 0.0f;
	//-------------------------------------------
	// Functions
	//-------------------------------------------
};