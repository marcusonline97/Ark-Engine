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
	Engine::GameObject* m_gameCamera = nullptr;
	std::unique_ptr<Engine::GameObject> m_editorCamera;
	Engine::SceneEditor m_sceneEditor;
	float m_editorCameraPitch = 0.0f;
	float m_editorCameraYaw = 0.0f;
	bool m_editorCameraLookActive = false;
	bool m_skipEditorCameraMouseDelta = false;

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
	void EnsureEditorCamera();
	void RestoreGameCamera();
	void SetEditorCameraLookActive(bool active);
	void SyncEditorCameraFrom(Engine::GameObject* camera);
	void UpdateEditorCamera(float deltaTime);
};