#pragma once

#include "Scene/Scene.h"

#include <array>
#include <memory>
#include <string>

namespace Engine
{
	class MeshComponent;

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
		void DrawMeshComponentFields(MeshComponent* meshComponent);
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
		MeshComponent* m_meshFieldComponent = nullptr;
		std::array<char, 256> m_meshPathBuffer = {};
		std::array<char, 256> m_materialPathBuffer = {};
	};
}