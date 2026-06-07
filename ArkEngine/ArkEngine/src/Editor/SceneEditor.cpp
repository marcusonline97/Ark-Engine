#include "SceneEditor.h"

#include "Scene/Components/LightComponent.h"
#include "Scene/Components/AudioComponent.h"
#include "Scene/Components/MeshComponent.h"
#include "Scene/Components/PhysicsComponent.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/AnimationComponent.h"
#include "Core/ArkEngine.h"
#include "Logger.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <GLAD/glad.h>
#include <algorithm>
#include <cctype>
#include <mutex>
#include <glm/gtc/quaternion.hpp>

namespace Engine
{
	namespace
	{
		constexpr const char* kAssetPathPayloadType = "ASSET_PATH";

		bool IsMeshAssetPayloadFile(const std::filesystem::path& p)
		{
			auto e = p.extension().string();
			for (auto& c : e) c = (char)std::tolower((unsigned char)c);
			return e == ".gltf" || e == ".glb" || e == ".obj";
		}
	}
	// ═══════════════════════════════════════════════════════════════════
	//  Public API
	// ═══════════════════════════════════════════════════════════════════

	void SceneEditor::SetScene(const std::shared_ptr<Scene>& scene, const std::string& scenePath)
	{
		m_scene = scene;
		m_scenePath = scenePath;
		m_dockspaceBuilt = false;   // rebuild layout for new scene
		ClearSelection();

		m_contentRoot = ArkEngine::GetInstance().GetFileSystem().GetAssetsFolder();
		m_contentCurrent = m_contentRoot;
		RefreshDirectory(m_contentCurrent);
	}

	// ── Local level-name helper (no Logger.cpp symbols needed) ─────────
	static const char* LevelNameLocal(Logging::Level level)
	{
		switch (level)
		{
		case Logging::Level::INIT:     return "INIT";
		case Logging::Level::DEBUG:    return "DEBUG";
		case Logging::Level::WARNING:  return "WARNING";
		case Logging::Level::_ERROR:   return "ERROR";
		case Logging::Level::FATAL:    return "FATAL";
		case Logging::Level::TODO:     return "TODO";
		case Logging::Level::FUNCTION: return "FUNCTION";
		default:                       return "LOG";
		}
	}

	// ── Static log-capture queue ─────────────────────────────────────
	// Lives entirely in this TU — zero Logger.cpp symbols required.
	// Logger::MessageStream::~MessageStream calls DispatchToSinks which
	// calls our sink lambda, which pushes into this queue.
	// DrawLogs() drains it on the main thread each frame.
	struct PendingLogEntry { Logging::Level level; std::string message; };
	struct LogQueue
	{
		std::mutex                   mtx;
		std::vector<PendingLogEntry> pending;
		SceneEditor* activeEditor = nullptr;
	};
	static LogQueue& GetLogQueue() { static LogQueue q; return q; }

	void SceneEditor::SetActive(bool active)
	{
		if (m_active == active) return;
		m_active = active;

		auto& q = GetLogQueue();

		if (active)
		{
			// Register a process-lifetime sink once; after that just set the owner.
			static bool s_sinkRegistered = false;
			if (!s_sinkRegistered)
			{
				s_sinkRegistered = true;
				Logging::AddSink([](Logging::Level level, std::string_view msg)
					{
						auto& q2 = GetLogQueue();
						std::lock_guard<std::mutex> lock(q2.mtx);
						if (!q2.activeEditor) return;
						std::string text(msg);
						if (!text.empty() && text.back() == '\n') text.pop_back();
						q2.pending.push_back({ level, std::move(text) });
					});
			}
			std::lock_guard<std::mutex> lock(q.mtx);
			q.activeEditor = this;
		}
		else
		{
			std::lock_guard<std::mutex> lock(q.mtx);
			if (q.activeEditor == this) q.activeEditor = nullptr;
		}
	}
	bool SceneEditor::IsActive() const { return m_active; }

	void SceneEditor::Render()
	{
		if (!m_active || !m_scene) return;

		DrawEditorDockspace();
		DrawViewport();
		DrawHierarchy();
		DrawInspector();
		DrawBottomPanel();
	}

	void SceneEditor::ClearSelection()
	{
		m_selectedObject = nullptr;
		m_nameBufferObject = nullptr;
		m_nameBuffer.fill('\0');
	}

	// ═══════════════════════════════════════════════════════════════════
	//  Dockspace
	// ═══════════════════════════════════════════════════════════════════

	void SceneEditor::DrawEditorDockspace()
	{
		ImGuiViewport* vp = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(vp->WorkPos);
		ImGui::SetNextWindowSize(vp->WorkSize);
		ImGui::SetNextWindowViewport(vp->ID);

		ImGuiWindowFlags hostFlags =
			ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::Begin("##EditorHost", nullptr, hostFlags);
		ImGui::PopStyleVar(3);

		ImGuiID dsID = ImGui::GetID("ArkEditorDockspace");
		ImGui::DockSpace(dsID, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);

		if (!m_dockspaceBuilt)
		{
			m_dockspaceBuilt = true;

			ImGui::DockBuilderRemoveNode(dsID);
			ImGui::DockBuilderAddNode(dsID, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dsID, vp->WorkSize);

			// left | rest
			ImGuiID leftID, restID;
			ImGui::DockBuilderSplitNode(dsID, ImGuiDir_Left, 0.20f, &leftID, &restID);

			// rest -> right | centre
			ImGuiID rightID, centreID;
			ImGui::DockBuilderSplitNode(restID, ImGuiDir_Right, 0.25f, &rightID, &centreID);

			// centre -> bottom (content browser) | viewport
			ImGuiID bottomID, viewID;
			ImGui::DockBuilderSplitNode(centreID, ImGuiDir_Down, 0.26f, &bottomID, &viewID);

			ImGui::DockBuilderDockWindow("Hierarchy", leftID);
			ImGui::DockBuilderDockWindow("Viewport", viewID);
			ImGui::DockBuilderDockWindow("Inspector", rightID);
			ImGui::DockBuilderDockWindow("Bottom Panel", bottomID);

			ImGui::DockBuilderFinish(dsID);
		}

		ImGui::End();
	}

	// ═══════════════════════════════════════════════════════════════════
	//  Viewport  –  shows the FBO colour texture + toolbar strip
	// ═══════════════════════════════════════════════════════════════════

	void SceneEditor::DrawViewport()
	{
		// Remove all padding so the image fills edge-to-edge
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport");
		ImGui::PopStyleVar();

		// ── Toolbar strip ────────────────────────────────────────
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.09f, 0.11f, 1.0f));
		ImGui::BeginChild("##VPToolbar", ImVec2(0, 30), false,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		ImGui::SetCursorPosY((30.0f - ImGui::GetTextLineHeight()) * 0.5f); // vertical centre
		ImGui::SetCursorPosX(8.0f);

		// Save button
		if (ImGui::Button("  Save  "))
		{
			SaveScene();
		}
		ImGui::SameLine(0, 12);

		// New-object name + add button
		ImGui::SetNextItemWidth(140.0f);
		ImGui::InputText("##NewName", m_newObjectName.data(), m_newObjectName.size());
		ImGui::SameLine(0, 4);
		if (ImGui::Button("Add Object"))
		{
			CreateObject(nullptr);
		}
		ImGui::SameLine(0, 20);

		// Scene path label
		ImGui::TextDisabled("%s", m_scenePath.c_str());
		ImGui::SameLine(0, 20);

		// Camera hint
		ImGui::TextDisabled("WASD fly | E/Space up | Q/Ctrl dn | Shift fast | RMB look");

		// Status message (right-aligned)
		if (!m_status.empty())
		{
			const float textW = ImGui::CalcTextSize(m_status.c_str()).x;
			const float avail = ImGui::GetContentRegionAvail().x;
			if (textW < avail - 8.0f)
			{
				ImGui::SameLine(0, avail - textW - 8.0f);
			}
			else
			{
				ImGui::SameLine(0, 8);
			}
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.49f, 0.51f, 1.0f, 1.0f));
			ImGui::TextUnformatted(m_status.c_str());
			ImGui::PopStyleColor();
		}

		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		// ── FBO image ────────────────────────────────────────────
		// Fill the rest of the Viewport window with the rendered scene
		ImVec2 viewSize = ImGui::GetContentRegionAvail();
		if (viewSize.x > 1 && viewSize.y > 1)
		{
			GLuint texID = ArkEngine::GetInstance().GetSceneColorTexture();
			if (texID)
			{
				// ImGui UV: OpenGL textures are stored bottom-up, so flip V
				ImGui::Image(
					(ImTextureID)(uintptr_t)texID,
					viewSize,
					ImVec2(0, 1),   // UV top-left  = (0,1) = bottom of GL texture
					ImVec2(1, 0)    // UV bot-right  = (1,0) = top of GL texture
				);
			}
		}

		ImGui::End();
	}

	// ═══════════════════════════════════════════════════════════════════
	//  Hierarchy
	// ═══════════════════════════════════════════════════════════════════

	void SceneEditor::DrawHierarchy()
	{
		ImGui::Begin("Hierarchy");

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.49f, 0.51f, 1.0f, 1.0f));
		ImGui::TextUnformatted("  World");
		ImGui::PopStyleColor();
		ImGui::Separator();

		for (const auto& obj : m_scene->GetRootObjects())
		{
			if (obj && obj->IsAlive())
				DrawObjectNode(obj.get());
		}

		if (ImGui::BeginPopupContextWindow("##HierCtx",
			ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create Empty Object")) CreateObject(nullptr);
			ImGui::EndPopup();
		}

		ImGui::End();
	}

	void SceneEditor::DrawObjectNode(GameObject* object)
	{
		if (!object || !object->IsAlive()) return;

		ImGuiTreeNodeFlags flags =
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			ImGuiTreeNodeFlags_OpenOnDoubleClick;

		if (object == m_selectedObject)         flags |= ImGuiTreeNodeFlags_Selected;
		if (object->GetChildren().empty())      flags |= ImGuiTreeNodeFlags_Leaf;

		if (!object->IsActive())
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

		const char* prefix = object->GetComponents().empty() ? "  " : "# ";
		const std::string label = std::string(prefix) + object->GetName();

		const bool open = ImGui::TreeNodeEx(
			reinterpret_cast<void*>(object), flags, "%s", label.c_str());

		if (!object->IsActive()) ImGui::PopStyleColor();

		if (ImGui::IsItemClicked()) SelectObject(object);

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Add Child")) CreateObject(object);
			ImGui::Separator();
			if (ImGui::MenuItem("Duplicate"))
			{
				auto* copy = m_scene->CreateObject(object->GetName() + " (copy)", object->GetParent());
				if (copy)
				{
					copy->SetPosition(object->GetPosition());
					copy->SetRotation(object->GetRotation());
					copy->SetScale(object->GetScale());
					SelectObject(copy);
					m_status = "Duplicated: " + object->GetName();
				}
			}
			ImGui::Separator();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
			if (ImGui::MenuItem("Delete")) { SelectObject(object); DeleteSelected(); }
			ImGui::PopStyleColor();
			ImGui::EndPopup();
		}

		if (open)
		{
			for (const auto& child : object->GetChildren())
				DrawObjectNode(child.get());
			ImGui::TreePop();
		}
	}

	// ═══════════════════════════════════════════════════════════════════
	//  Inspector
	// ═══════════════════════════════════════════════════════════════════

	void SceneEditor::DrawInspector()
	{
		ImGui::Begin("Inspector");

		if (!m_selectedObject || !m_selectedObject->IsAlive())
		{
			ImGui::Spacing();
			ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - 200.0f) * 0.5f);
			ImGui::TextDisabled("Select an object in the Hierarchy");
			ImGui::End();
			return;
		}

		// Sync name buffer
		if (m_nameBufferObject != m_selectedObject)
		{
			m_nameBuffer.fill('\0');
			const auto& n = m_selectedObject->GetName();
			std::copy_n(n.c_str(), std::min(n.size(), m_nameBuffer.size() - 1), m_nameBuffer.data());
			m_nameBufferObject = m_selectedObject;
		}

		// Header
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.49f, 0.51f, 1.0f, 1.0f));
		ImGui::TextUnformatted("  Entity");
		ImGui::PopStyleColor();
		ImGui::Separator();
		ImGui::Spacing();

		// Active + name row
		bool active = m_selectedObject->IsActive();
		if (ImGui::Checkbox("##active", &active)) m_selectedObject->SetActive(active);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-1.0f);
		if (ImGui::InputText("##Name", m_nameBuffer.data(), m_nameBuffer.size()))
			m_selectedObject->SetName(m_nameBuffer.data());

		ImGui::Spacing();

		// Transform
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::CollapsingHeader("  Transform"))
		{
			ImGui::Spacing();
			constexpr float labelW = 80.0f;

			glm::vec3 pos = m_selectedObject->GetPosition();
			float p[3] = { pos.x, pos.y, pos.z };
			ImGui::Text("Position"); ImGui::SameLine(labelW);
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::DragFloat3("##Pos", p, 0.1f))
				m_selectedObject->SetPosition({ p[0], p[1], p[2] });

			glm::quat rot = m_selectedObject->GetRotation();
			float r[4] = { rot.x, rot.y, rot.z, rot.w };
			ImGui::Text("Rotation"); ImGui::SameLine(labelW);
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::DragFloat4("##Rot", r, 0.01f))
				m_selectedObject->SetRotation(glm::normalize(glm::quat(r[3], r[0], r[1], r[2])));

			glm::vec3 scl = m_selectedObject->GetScale();
			float s[3] = { scl.x, scl.y, scl.z };
			ImGui::Text("Scale"); ImGui::SameLine(labelW);
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::DragFloat3("##Scl", s, 0.05f))
				m_selectedObject->SetScale({ s[0], s[1], s[2] });

			ImGui::Spacing();
		}

		DrawComponents(m_selectedObject);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		const float btnW = (ImGui::GetContentRegionAvail().x -
			ImGui::GetStyle().ItemSpacing.x) * 0.5f;
		if (ImGui::Button("Add Child Object", ImVec2(btnW, 0)))
			CreateObject(m_selectedObject);
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.1f, 0.1f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.85f, 0.1f, 0.1f, 1.0f));
		if (ImGui::Button("Delete Object", ImVec2(btnW, 0))) DeleteSelected();
		ImGui::PopStyleColor(3);

		ImGui::End();
	}

	void SceneEditor::DrawComponents(GameObject* object)
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (!ImGui::CollapsingHeader("  Components")) return;

		ImGui::Spacing();

		Component* toRemove = nullptr;

		for (const auto& comp : object->GetComponents())
		{
			if (!comp) continue;

			const std::string typeName =
				ComponentFactory::GetInstance().GetTypeName(comp->GetTypeId());

			ImGui::PushID(comp.get());

			// Purple accent bar on the left of each node
			const ImVec2 nodeMin = ImGui::GetCursorScreenPos();
			const bool nodeOpen = ImGui::TreeNode(typeName.c_str());
			ImGui::GetWindowDrawList()->AddRectFilled(
				ImVec2(nodeMin.x - 4.0f, nodeMin.y + 2.0f),
				ImVec2(nodeMin.x - 1.0f, nodeMin.y + ImGui::GetTextLineHeight() - 2.0f),
				IM_COL32(126, 130, 255, 200));

			if (nodeOpen)
			{
				ImGui::Spacing();

				if (auto* light = dynamic_cast<LightComponent*>(comp.get()))
				{
					glm::vec3 col = light->GetColor();
					float c[3] = { col.r, col.g, col.b };
					if (ImGui::ColorEdit3("Color", c))
						light->SetColor({ c[0], c[1], c[2] });
				}
				else if (dynamic_cast<CameraComponent*>(comp.get()))
				{
					ImGui::TextDisabled("FOV / near / far configured in code.");
				}
				else if (dynamic_cast<AudioComponent*>(comp.get()))
				{
					ImGui::TextDisabled("Audio clips loaded from JSON.");
				}

				else if (auto* meshComp = dynamic_cast<MeshComponent*>(comp.get()))
				{
					const std::string & meshPath = meshComp->GetMeshPath();
					std::vector<char> meshPathBuffer(meshPath.begin(), meshPath.end());
					meshPathBuffer.push_back('\0');

					ImGui::TextUnformatted("Current Mesh Path");
					ImGui::SetNextItemWidth(-1.0f);
					ImGui::InputText("##MeshPath", meshPathBuffer.data(), meshPathBuffer.size(), ImGuiInputTextFlags_ReadOnly);

					ImGui::Button("Mesh", ImVec2(-1.0f, 0.0f));

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kAssetPathPayloadType))
						{
							const char* data = static_cast<const char*>(payload->Data);
							if (data && payload->DataSize > 0)
							{
								size_t length = 0;
								while (length < static_cast<size_t>(payload->DataSize) && data[length] != '\0')
								{
									++length;
								}
								std::string path(data, length);
								auto newMesh = ArkEngine::GetInstance().GetMeshManager().GetOrLoadMesh(path);
								if (newMesh)
								{
									meshComp->SetMesh(newMesh);
									meshComp->SetMeshPath(path);
									m_status = "Assigned mesh: " + path;
								}
								else
								{
									m_status = "Failed to load mesh: " + path;
								}
							}
						}
						ImGui::EndDragDropTarget();

						//Might need to move the EndDragDropTarget here -> Todo:
					}

				}

				else if (auto* anim = dynamic_cast<AnimationComponent*>(comp.get()))
				{
					ImGui::TextDisabled("Playing: %s", anim->IsPlaying() ? "yes" : "no");
				}
				else
				{
					ImGui::TextDisabled("No editable fields — preserved in JSON.");
				}

				ImGui::Spacing();
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.45f, 0.08f, 0.08f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.65f, 0.15f, 0.15f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.75f, 0.08f, 0.08f, 1.0f));
				if (ImGui::Button("Remove Component")) toRemove = comp.get();
				ImGui::PopStyleColor(3);
				ImGui::Spacing();
				ImGui::TreePop();
			}
			ImGui::PopID();
		}

		if (toRemove) object->RemoveComponent(toRemove);

		// Add component
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		const auto& names = ComponentFactory::GetInstance().GetRegisteredNames();
		if (!names.empty())
		{
			m_componentTypeIndex = std::clamp(m_componentTypeIndex, 0, (int)names.size() - 1);
			ImGui::SetNextItemWidth(-80.0f);
			if (ImGui::BeginCombo("##CompType", names[m_componentTypeIndex].c_str()))
			{
				for (int i = 0; i < (int)names.size(); ++i)
				{
					const bool sel = (i == m_componentTypeIndex);
					if (ImGui::Selectable(names[i].c_str(), sel)) m_componentTypeIndex = i;
					if (sel) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			if (ImGui::Button("Add##Comp", ImVec2(-1.0f, 0.0f)))
			{
				const std::string& cname = names[m_componentTypeIndex];
				if (Component* c = ComponentFactory::GetInstance().CreateComponent(cname))
				{
					c->SetSerializedData({ {"type", cname} });
					object->AddComponent(c);
					m_status = "Added component: " + cname;
				}
			}
		}
	}

	// ═══════════════════════════════════════════════════════════════════
	//  Bottom Panel  –  tabbed: Content Browser | Rendering | Logs
	// ═══════════════════════════════════════════════════════════════════

	void SceneEditor::DrawBottomPanel()
	{
		ImGui::Begin("Bottom Panel");

		if (ImGui::BeginTabBar("##BottomTabs"))
		{
			if (ImGui::BeginTabItem("Content Browser"))
			{
				DrawContentBrowser();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Rendering"))
			{
				DrawRendering();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Logs"))
			{
				DrawLogs();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	// ═══════════════════════════════════════════════════════════════════
	//  Content Browser
	// ═══════════════════════════════════════════════════════════════════

	const char* SceneEditor::FileIcon(const std::filesystem::path& p)
	{
		if (std::filesystem::is_directory(p)) return "[DIR]";
		if (IsImageFile(p))                   return "[IMG]";
		if (IsMaterialFile(p))                return "[MAT]";
		if (IsSceneFile(p))                   return "[SCN]";
		if (IsAudioFile(p))                   return "[SFX]";
		if (IsMeshFile(p))                    return "[MSH]";
		return "[FILE]";
	}

	bool SceneEditor::IsImageFile(const std::filesystem::path& p)
	{
		auto e = p.extension().string();
		for (auto& c : e) c = (char)std::tolower((unsigned char)c);
		return e == ".png" || e == ".jpg" || e == ".jpeg" || e == ".tga" || e == ".bmp";
	}
	bool SceneEditor::IsMaterialFile(const std::filesystem::path& p)
	{
		auto e = p.extension().string();
		return e == ".mat" || e == ".material";
	}
	bool SceneEditor::IsSceneFile(const std::filesystem::path& p)
	{
		auto e = p.extension().string();
		return e == ".sc" || e == ".scene" || e == ".json";
	}
	bool SceneEditor::IsAudioFile(const std::filesystem::path& p)
	{
		auto e = p.extension().string();
		for (auto& c : e) c = (char)std::tolower((unsigned char)c);
		return e == ".wav" || e == ".mp3" || e == ".ogg" || e == ".flac";
	}
	bool SceneEditor::IsMeshFile(const std::filesystem::path& p)
	{
		auto e = p.extension().string();
		for (auto& c : e) c = (char)std::tolower((unsigned char)c);
		return e == ".gltf" || e == ".glb" || e == ".obj" || e == ".fbx";
	}

	void SceneEditor::RefreshDirectory(const std::filesystem::path& dir)
	{
		m_contentEntries.clear();
		if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) return;

		std::vector<std::filesystem::path> dirs, files;
		for (auto& entry : std::filesystem::directory_iterator(dir))
		{
			if (entry.is_directory()) dirs.push_back(entry.path());
			else                      files.push_back(entry.path());
		}
		std::sort(dirs.begin(), dirs.end());
		std::sort(files.begin(), files.end());
		for (auto& d : dirs)  m_contentEntries.push_back(d);
		for (auto& f : files) m_contentEntries.push_back(f);
	}

	void SceneEditor::DrawContentBrowser()
	{
		// Top bar
		const bool canGoUp = (m_contentCurrent != m_contentRoot);
		if (!canGoUp) ImGui::BeginDisabled();
		if (ImGui::Button("< Back"))
		{
			m_contentCurrent = m_contentCurrent.parent_path();
			RefreshDirectory(m_contentCurrent);
			m_contentFilterBuf.fill('\0');
			m_contentFilter.clear();
		}
		if (!canGoUp) ImGui::EndDisabled();

		ImGui::SameLine();
		auto rel = std::filesystem::relative(m_contentCurrent, m_contentRoot);
		std::string breadcrumb = "Assets";
		if (rel != "." && !rel.empty())
			for (const auto& part : rel) breadcrumb += " / " + part.string();

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.49f, 0.51f, 1.0f, 1.0f));
		ImGui::TextUnformatted(breadcrumb.c_str());
		ImGui::PopStyleColor();

		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 200.0f);
		ImGui::SetNextItemWidth(200.0f);
		if (ImGui::InputTextWithHint("##Filter", "Filter...",
			m_contentFilterBuf.data(), m_contentFilterBuf.size()))
		{
			m_contentFilter = m_contentFilterBuf.data();
			for (auto& c : m_contentFilter) c = (char)std::tolower((unsigned char)c);
		}

		ImGui::Separator();

		// Grid
		constexpr float itemW = 88.0f;
		constexpr float itemH = 72.0f;
		const float     panelW = ImGui::GetContentRegionAvail().x;
		const int       cols = std::max(1, (int)(panelW / (itemW + 8.0f)));
		ImGui::Columns(cols, nullptr, false);

		for (const auto& entry : m_contentEntries)
		{
			const bool isDir = std::filesystem::is_directory(entry);
			std::string fname = entry.filename().string();

			if (!m_contentFilter.empty())
			{
				std::string lower = fname;
				for (auto& c : lower) c = (char)std::tolower((unsigned char)c);
				if (lower.find(m_contentFilter) == std::string::npos)
				{
					ImGui::NextColumn(); continue;
				}
			}

			ImGui::PushID(fname.c_str());

			ImVec4 iconCol = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
			if (isDir)                   iconCol = ImVec4(0.98f, 0.82f, 0.37f, 1.0f);
			else if (IsImageFile(entry)) iconCol = ImVec4(0.45f, 0.85f, 0.65f, 1.0f);
			else if (IsMaterialFile(entry)) iconCol = ImVec4(0.55f, 0.60f, 1.00f, 1.0f);
			else if (IsSceneFile(entry)) iconCol = ImVec4(1.00f, 0.65f, 0.25f, 1.0f);
			else if (IsAudioFile(entry)) iconCol = ImVec4(0.90f, 0.45f, 0.90f, 1.0f);
			else if (IsMeshFile(entry))  iconCol = ImVec4(0.40f, 0.80f, 1.00f, 1.0f);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.13f, 0.15f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.18f, 0.54f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.24f, 0.22f, 0.60f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Text, iconCol);

			const std::string btnLabel = std::string(FileIcon(entry)) + "\n" + fname;
			std::string relPath = std::filesystem::relative(entry, m_contentRoot).string();
			std::replace(relPath.begin(), relPath.end(), '\\', '/');
			if (ImGui::Button(btnLabel.c_str(), ImVec2(itemW, itemH)))
			{
				if (isDir)
				{
					m_contentCurrent = entry;
					RefreshDirectory(m_contentCurrent);
					m_contentFilterBuf.fill('\0');
					m_contentFilter.clear();
				}
				else
				{
					m_status = "Selected: " + relPath;
				}
			}
			ImGui::PopStyleColor(4);

			if (!isDir && IsMeshAssetPayloadFile(entry) && ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload(kAssetPathPayloadType, relPath.c_str(), relPath.size() + 1);
				ImGui::TextUnformatted(relPath.c_str());
				ImGui::EndDragDropSource();
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", relPath.c_str());
			}

			ImGui::NextColumn();
			ImGui::PopID();
		}

		ImGui::Columns(1);
		if (m_contentEntries.empty())
		{
			ImGui::Spacing();
			ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - 120.0f) * 0.5f);
			ImGui::TextDisabled("(folder is empty)");
		}
	}

	// ═══════════════════════════════════════════════════════════════════
	//  Rendering panel
	// ═══════════════════════════════════════════════════════════════════

	void SceneEditor::DrawRendering()
	{
		ImGui::Spacing();

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.49f, 0.51f, 1.0f, 1.0f));
		ImGui::TextUnformatted("  Textures");
		ImGui::PopStyleColor();
		ImGui::Separator();
		ImGui::Spacing();

		const char* filterNames[] = { "Linear", "Nearest" };
		int current = static_cast<int>(m_mipmapFilter);

		ImGui::Text("Mipmap");
		ImGui::SameLine(90.0f);
		ImGui::SetNextItemWidth(120.0f);
		if (ImGui::Combo("##MipmapFilter", &current, filterNames, IM_ARRAYSIZE(filterNames)))
		{
			m_mipmapFilter = static_cast<MipmapFilter>(current);
			ApplyMipmapFilter(m_mipmapFilter);
			m_status = std::string("Mipmap: ") + filterNames[current];
		}

		ImGui::SameLine(0, 12);
		ImGui::TextDisabled("applied to all loaded asset textures");
		ImGui::Spacing();
	}

	void SceneEditor::ApplyMipmapFilter(MipmapFilter filter)
	{
		const GLint minFilter = (filter == MipmapFilter::Linear)
			? GL_LINEAR_MIPMAP_LINEAR
			: GL_NEAREST_MIPMAP_NEAREST;
		const GLint magFilter = (filter == MipmapFilter::Linear)
			? GL_LINEAR
			: GL_NEAREST;

		// Route through TextureManager so we only touch asset textures —
		// never the FBO colour attachment, depth buffer, or font atlas.
		ArkEngine::GetInstance().GetTextureManager().SetFilterOnAllTextures(minFilter, magFilter);
	}

	// ═══════════════════════════════════════════════════════════════════
	//  Logs panel
	// ═══════════════════════════════════════════════════════════════════

	void SceneEditor::DrawLogs()
	{
		// Drain pending messages captured by the sink lambda
		{
			auto& q = GetLogQueue();
			std::lock_guard<std::mutex> lock(q.mtx);
			constexpr std::size_t k_max = 2000;
			for (auto& p : q.pending)
			{
				if (m_logEntries.size() >= k_max)
					m_logEntries.erase(m_logEntries.begin());
				m_logEntries.push_back({ p.level, std::move(p.message) });
			}
			q.pending.clear();
		}

		// Toolbar
		if (ImGui::Button("Clear")) m_logEntries.clear();
		ImGui::SameLine();
		ImGui::Checkbox("Auto-scroll", &m_logAutoScroll);
		ImGui::SameLine(0, 20);
		ImGui::Checkbox("Init", &m_logShowInit);    ImGui::SameLine();
		ImGui::Checkbox("Debug", &m_logShowDebug);   ImGui::SameLine();
		ImGui::Checkbox("Warning", &m_logShowWarning); ImGui::SameLine();
		ImGui::Checkbox("Error", &m_logShowError);   ImGui::SameLine();
		ImGui::Checkbox("Fatal", &m_logShowFatal);
		ImGui::Separator();

		ImGui::BeginChild("##LogScroll", ImVec2(0, 0), false,
			ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));

		for (const auto& entry : m_logEntries)
		{
			switch (entry.level)
			{
			case Logging::Level::INIT:    if (!m_logShowInit)    continue; break;
			case Logging::Level::DEBUG:   if (!m_logShowDebug)   continue; break;
			case Logging::Level::WARNING: if (!m_logShowWarning) continue; break;
			case Logging::Level::_ERROR:  if (!m_logShowError)   continue; break;
			case Logging::Level::FATAL:   if (!m_logShowFatal)   continue; break;
			default: break;
			}

			ImVec4 col = { 1,1,1,1 };
			switch (entry.level)
			{
			case Logging::Level::INIT:    col = { 0.70f, 0.40f, 0.90f, 1 }; break;
			case Logging::Level::DEBUG:   col = { 0.40f, 0.80f, 0.90f, 1 }; break;
			case Logging::Level::WARNING: col = { 0.95f, 0.80f, 0.20f, 1 }; break;
			case Logging::Level::_ERROR:  col = { 0.95f, 0.30f, 0.30f, 1 }; break;
			case Logging::Level::FATAL:   col = { 1.00f, 0.10f, 0.10f, 1 }; break;
			default: break;
			}

			ImGui::PushStyleColor(ImGuiCol_Text, col);
			const std::string line = std::string("[") + LevelNameLocal(entry.level) + "] " + entry.message;
			ImGui::TextUnformatted(line.c_str());
			ImGui::PopStyleColor();
		}

		ImGui::PopStyleVar();
		if (m_logAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);
		ImGui::EndChild();
	}

	// ═══════════════════════════════════════════════════════════════════
	//  Helpers
	// ═══════════════════════════════════════════════════════════════════

	void SceneEditor::SelectObject(GameObject* object)
	{
		m_selectedObject = object;
		m_nameBufferObject = nullptr;
	}

	GameObject* SceneEditor::CreateObject(GameObject* parent)
	{
		if (!m_scene) return nullptr;
		std::string name = m_newObjectName.data();
		if (name.empty()) name = "New Object";
		GameObject* obj = m_scene->CreateObject(name, parent);
		if (obj)
		{
			obj->SetSerializedData({ {"name", name} });
			SelectObject(obj);
			m_status = "Created: " + name;
		}
		return obj;
	}

	void SceneEditor::DeleteSelected()
	{
		if (!m_selectedObject) return;
		m_status = "Deleted: " + m_selectedObject->GetName();
		m_selectedObject->MarkForDestroy();
		ClearSelection();
	}

	void SceneEditor::SaveScene()
	{
		if (!m_scene) return;
		m_status = m_scene->Save(m_scenePath)
			? "Saved  " + m_scenePath
			: "FAILED to save  " + m_scenePath;
	}
}