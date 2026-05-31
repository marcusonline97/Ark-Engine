#include "SceneEditor.h"

#include "Scene/Components/LightComponent.h"
#include "imgui/imgui.h"

#include <algorithm>
#include <glm/gtc/quaternion.hpp>

namespace Engine
{
	void SceneEditor::SetScene(const std::shared_ptr<Scene>& scene, const std::string& scenePath)
	{
		m_scene = scene;
		m_scenePath = scenePath;
		ClearSelection();
	}

	void SceneEditor::SetActive(bool active)
	{
		m_active = active;
	}

	bool SceneEditor::IsActive() const
	{
		return m_active;
	}

	void SceneEditor::Render()
	{
		if (!m_active || !m_scene)
		{
			return;
		}

		DrawToolbar();
		DrawHierarchy();
		DrawInspector();
	}

	void SceneEditor::ClearSelection()
	{
		m_selectedObject = nullptr;
		m_nameBufferObject = nullptr;
		std::fill(m_nameBuffer.begin(), m_nameBuffer.end(), '\0');
	}

	void SceneEditor::DrawToolbar()
	{
		ImGui::Begin("Editor");
		ImGui::Text("Editing: %s", m_scenePath.c_str());
		ImGui::TextUnformatted("Camera: hold RMB, use WASD to fly, E/Space up, Q/Ctrl down, Shift to speed up");
		if (ImGui::Button("Save Scene"))
		{
			SaveScene();
		}
		ImGui::SameLine();
		if (ImGui::Button("Add Root Object"))
		{
			CreateObject(nullptr);
		}
		ImGui::InputText("New Object Name", m_newObjectName.data(), m_newObjectName.size());
		if (!m_status.empty())
		{
			ImGui::Separator();
			ImGui::TextWrapped("%s", m_status.c_str());
		}
		ImGui::End();
	}

	void SceneEditor::DrawHierarchy()
	{
		ImGui::Begin("World Inspector");

		for (const auto& object : m_scene->GetRootObjects())
		{
			if (object && object->IsAlive())
			{
				DrawObjectNode(object.get());
			}
		}

		ImGui::End();
	}

	void SceneEditor::DrawObjectNode(GameObject* object)
	{
		if (!object || !object->IsAlive())
		{
			return;
		}

		ImGuiTreeNodeFlags flags =
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_SpanAvailWidth;

		if (object == m_selectedObject)
		{
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		if (object->GetChildren().empty())
		{
			flags |= ImGuiTreeNodeFlags_Leaf;
		}

		const bool open = ImGui::TreeNodeEx(
			reinterpret_cast<void*>(object),
			flags,
			"%s",
			object->GetName().c_str());

		if (ImGui::IsItemClicked())
		{
			SelectObject(object);
		}

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Add Child"))
			{
				CreateObject(object);
			}
			if (ImGui::MenuItem("Delete"))
			{
				SelectObject(object);
				DeleteSelected();
			}
			ImGui::EndPopup();
		}

		if (open)
		{
			for (const auto& child : object->GetChildren())
			{
				DrawObjectNode(child.get());
			}
			ImGui::TreePop();
		}
	}

	void SceneEditor::DrawInspector()
	{
		ImGui::Begin("Inspector");

		if (!m_selectedObject || !m_selectedObject->IsAlive())
		{
			ImGui::TextUnformatted("Select an object in the World Inspector.");
			ImGui::End();
			return;
		}

		if (m_nameBufferObject != m_selectedObject)
		{
			std::fill(m_nameBuffer.begin(), m_nameBuffer.end(), '\0');
			const auto& objectName = m_selectedObject->GetName();
			std::copy_n(
				objectName.c_str(),
				std::min(objectName.size(), m_nameBuffer.size() - 1),
				m_nameBuffer.data());
			m_nameBufferObject = m_selectedObject;
		}

		if (ImGui::InputText("Name", m_nameBuffer.data(), m_nameBuffer.size()))
		{
			m_selectedObject->SetName(m_nameBuffer.data());
		}

		bool active = m_selectedObject->IsActive();
		if (ImGui::Checkbox("Active", &active))
		{
			m_selectedObject->SetActive(active);
		}

		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			glm::vec3 position = m_selectedObject->GetPosition();
			float pos[3] = { position.x, position.y, position.z };
			if (ImGui::DragFloat3("Position", pos, 0.1f))
			{
				m_selectedObject->SetPosition(glm::vec3(pos[0], pos[1], pos[2]));
			}

			glm::quat rotation = m_selectedObject->GetRotation();
			float rot[4] = { rotation.x, rotation.y, rotation.z, rotation.w };
			if (ImGui::DragFloat4("Rotation (x y z w)", rot, 0.01f))
			{
				m_selectedObject->SetRotation(glm::normalize(glm::quat(rot[3], rot[0], rot[1], rot[2])));
			}

			glm::vec3 scale = m_selectedObject->GetScale();
			float scl[3] = { scale.x, scale.y, scale.z };
			if (ImGui::DragFloat3("Scale", scl, 0.1f))
			{
				m_selectedObject->SetScale(glm::vec3(scl[0], scl[1], scl[2]));
			}
		}

		DrawComponents(m_selectedObject);

		if (ImGui::Button("Add Child Object"))
		{
			CreateObject(m_selectedObject);
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete Object"))
		{
			DeleteSelected();
		}

		ImGui::End();
	}

	void SceneEditor::DrawComponents(GameObject* object)
	{
		if (!ImGui::CollapsingHeader("Components", ImGuiTreeNodeFlags_DefaultOpen))
		{
			return;
		}

		Component* componentToRemove = nullptr;
		for (const auto& component : object->GetComponents())
		{
			if (!component)
			{
				continue;
			}

			const std::string typeName = ComponentFactory::GetInstance().GetTypeName(component->GetTypeId());
			ImGui::PushID(component.get());
			if (ImGui::TreeNode(typeName.c_str()))
			{
				if (auto light = dynamic_cast<LightComponent*>(component.get()))
				{
					glm::vec3 color = light->GetColor();
					float col[3] = { color.r, color.g, color.b };
					if (ImGui::ColorEdit3("Color", col))
					{
						light->SetColor(glm::vec3(col[0], col[1], col[2]));
					}
				}
				else
				{
					ImGui::TextWrapped("This component is preserved in JSON and can be removed, but does not have custom editor fields yet.");
				}

				if (ImGui::Button("Remove Component"))
				{
					componentToRemove = component.get();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}

		if (componentToRemove)
		{
			object->RemoveComponent(componentToRemove);
		}

		const auto& componentNames = ComponentFactory::GetInstance().GetRegisteredNames();
		if (!componentNames.empty())
		{
			m_componentTypeIndex = std::clamp(
				m_componentTypeIndex,
				0,
				static_cast<int>(componentNames.size()) - 1);

			const char* preview = componentNames[m_componentTypeIndex].c_str();
			if (ImGui::BeginCombo("Component Type", preview))
			{
				for (int i = 0; i < static_cast<int>(componentNames.size()); ++i)
				{
					const bool selected = i == m_componentTypeIndex;
					if (ImGui::Selectable(componentNames[i].c_str(), selected))
					{
						m_componentTypeIndex = i;
					}
					if (selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			if (ImGui::Button("Add Component"))
			{
				const std::string& componentName = componentNames[m_componentTypeIndex];
				Component* component = ComponentFactory::GetInstance().CreateComponent(componentName);
				if (component)
				{
					component->SetSerializedData({ {"type", componentName} });
					object->AddComponent(component);
					m_status = "Added component: " + componentName;
				}
			}
		}
	}

	void SceneEditor::SelectObject(GameObject* object)
	{
		m_selectedObject = object;
		m_nameBufferObject = nullptr;
	}

	GameObject* SceneEditor::CreateObject(GameObject* parent)
	{
		if (!m_scene)
		{
			return nullptr;
		}

		std::string name = m_newObjectName.data();
		if (name.empty())
		{
			name = "New Object";
		}

		GameObject* object = m_scene->CreateObject(name, parent);
		if (object)
		{
			object->SetSerializedData({ {"name", name} });
			SelectObject(object);
			m_status = "Created object: " + name;
		}

		return object;
	}

	void SceneEditor::DeleteSelected()
	{
		if (!m_selectedObject)
		{
			return;
		}

		m_selectedObject->MarkForDestroy();
		m_status = "Deleted object: " + m_selectedObject->GetName();
		ClearSelection();
	}

	void SceneEditor::SaveScene()
	{
		if (!m_scene)
		{
			return;
		}

		m_status = m_scene->Save(m_scenePath)
			? "Saved " + m_scenePath
			: "Failed to save " + m_scenePath;
	}
}