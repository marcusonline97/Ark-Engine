#pragma once

#include "Core/EngineInclude.h"
#include "Editor/SceneEditor.h"

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
	void RenderUI() override;

	void Destroy() override;

private:
	//-------------------------------------------
	// Properties & Variables
	//-------------------------------------------
	std::shared_ptr<Engine::Scene> m_scene;
	Engine::GameObject* m_3DRoot = nullptr;
	Engine::GameObject* m_mainPlayer = nullptr;
	Engine::SceneEditor m_sceneEditor;

	enum class Mode
	{
		Menu,
		Playing,
		Editing
	};

	Mode m_mode = Mode::Menu;

	//-------------------------------------------
	// Functions
	//-------------------------------------------
	void EnterMenuMode();
	void EnterPlayMode();
	void EnterEditMode();
};