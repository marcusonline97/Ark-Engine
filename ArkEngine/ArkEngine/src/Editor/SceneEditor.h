#pragma once

#include "Scene/Scene.h"

#include <array>
#include <memory>
#include <string>

namespace Engine
{
	class SceneEditor
	{
	public:
		void SetScene(const std::shared_ptr<Scene>& scene, const std::string& scenePath);

		void SetActive(bool active);

		bool IsActive() const;

		void Render();

		void ClearSelection();

	private:
		void DrawToolbar();
		void DrawHierarchy();
		void DrawObjectNode(GameObject* object);
		void DrawInspector();
		void DrawComponents(GameObject* object);
		void SelectObject(GameObject* object);
		GameObject* CreateObject(GameObject* parent);
		void DeleteSelected();
		void SaveScene();

	private:
		std::shared_ptr<Scene> m_scene;
		std::string m_scenePath = "Scenes/scene.sc";
		GameObject* m_selectedObject = nullptr;
		GameObject* m_nameBufferObject = nullptr;
		bool m_active = false;
		int m_componentTypeIndex = 0;
		std::string m_status;
		std::array<char, 128> m_nameBuffer = {};
		std::array<char, 128> m_newObjectName = { 'N', 'e', 'w', ' ', 'O', 'b', 'j', 'e', 'c', 't', '\0' };
	};
}