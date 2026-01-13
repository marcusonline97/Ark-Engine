#pragma once

#include <algorithm>
#include <cctype>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include <imgui/imgui.h>

#include "Logger.h"

namespace Ark::Editor
{
	// Extracted from EditorUI to keep the main editor class focused on layout + orchestration.
	class ConsolePanel
	{
	public:
		void PushLog(Logging::Level level, std::string_view msg)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_entries.push_back(Entry{ level, std::string(msg) });
		}

		void Clear()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_entries.clear();
		}

		void Render()
		{
			if (!ImGui::Begin("Console"))
			{
				ImGui::End();
				return;
			}

			if (ImGui::Button("Clear"))
				Clear();

			ImGui::SameLine();
			ImGui::Checkbox("Auto-scroll", &m_autoScroll);
			ImGui::SameLine();
			ImGui::Checkbox("Wrap", &m_wrap);

			ImGui::Separator();
			ImGui::SetNextItemWidth(280.0f);
			ImGui::InputTextWithHint("##consoleFilter", "Filter text...", m_filter, sizeof(m_filter));

			ImGui::SameLine();
			ImGui::TextUnformatted("Levels:");
			ImGui::SameLine(); ImGui::Checkbox("INFO", &m_showInfo);
			ImGui::SameLine(); ImGui::Checkbox("DBG", &m_showDebug);
			ImGui::SameLine(); ImGui::Checkbox("WRN", &m_showWarning);
			ImGui::SameLine(); ImGui::Checkbox("ERR", &m_showError);
			ImGui::SameLine(); ImGui::Checkbox("FTL", &m_showFatal);
			ImGui::SameLine(); ImGui::Checkbox("TODO", &m_showTodo);
			ImGui::SameLine(); ImGui::Checkbox("FUNC", &m_showFunction);

			ImGui::Separator();

			ImGuiWindowFlags childFlags = ImGuiWindowFlags_HorizontalScrollbar;
			ImGui::BeginChild("##consoleScroll", ImVec2(0, 0), false, childFlags);

			const std::string_view filterView = m_filter;
			std::vector<Entry> snapshot;
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				snapshot = m_entries;
			}

			for (const auto& e : snapshot)
			{
				if (!PassesLevelFilter(e.level))
					continue;
				if (!CaseInsensitiveFind(e.message, filterView))
					continue;

				ImGui::PushStyleColor(ImGuiCol_Text, LevelColor(e.level));
				ImGui::Text("[%s] ", LevelName(e.level));
				ImGui::PopStyleColor();
				ImGui::SameLine();

				if (m_wrap)
					ImGui::TextWrapped("%s", e.message.c_str());
				else
					ImGui::TextUnformatted(e.message.c_str());
			}

			if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f)
				ImGui::SetScrollHereY(1.0f);

			ImGui::EndChild();
			ImGui::End();
		}

	private:
		struct Entry
		{
			Logging::Level level{};
			std::string message;
		};

		static const char* LevelName(Logging::Level level)
		{
			switch (level)
			{
			case Logging::Level::INIT: return "INIT";
			case Logging::Level::_ERROR: return "ERROR";
			case Logging::Level::WARNING: return "WARN";
			case Logging::Level::DEBUG: return "DEBUG";
			case Logging::Level::FATAL: return "FATAL";
			case Logging::Level::TODO: return "TODO";
			case Logging::Level::FUNCTION: return "FUNC";
			}
			return "LOG";
		}

		static ImVec4 LevelColor(Logging::Level level)
		{
			switch (level)
			{
			case Logging::Level::_ERROR:   return ImVec4(0.95f, 0.25f, 0.25f, 1.0f);
			case Logging::Level::WARNING:  return ImVec4(0.95f, 0.75f, 0.20f, 1.0f);
			case Logging::Level::DEBUG:    return ImVec4(0.35f, 0.80f, 0.95f, 1.0f);
			case Logging::Level::FATAL:    return ImVec4(1.00f, 0.10f, 0.10f, 1.0f);
			case Logging::Level::TODO:     return ImVec4(0.35f, 0.95f, 0.35f, 1.0f);
			case Logging::Level::FUNCTION: return ImVec4(0.45f, 0.55f, 1.00f, 1.0f);
			case Logging::Level::INIT:     return ImVec4(0.80f, 0.80f, 0.80f, 1.0f);
			}
			return ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
		}

		bool PassesLevelFilter(Logging::Level lvl) const
		{
			switch (lvl)
			{
			case Logging::Level::DEBUG:    return m_showDebug;
			case Logging::Level::INIT:     return m_showInfo;
			case Logging::Level::WARNING:  return m_showWarning;
			case Logging::Level::_ERROR:   return m_showError;
			case Logging::Level::FATAL:    return m_showFatal;
			case Logging::Level::TODO:     return m_showTodo;
			case Logging::Level::FUNCTION: return m_showFunction;
			}
			return true;
		}

		static bool CaseInsensitiveFind(std::string_view haystack, std::string_view needle)
		{
			if (needle.empty()) return true;
			if (haystack.size() < needle.size()) return false;

			auto lower = [](unsigned char c) { return static_cast<char>(std::tolower(c)); };

			for (size_t i = 0; i + needle.size() <= haystack.size(); ++i)
			{
				bool ok = true;
				for (size_t j = 0; j < needle.size(); ++j)
				{
					if (lower(static_cast<unsigned char>(haystack[i + j])) != lower(static_cast<unsigned char>(needle[j])))
					{
						ok = false;
						break;
					}
				}
				if (ok) return true;
			}
			return false;
		}

	private:
		std::mutex m_mutex;
		std::vector<Entry> m_entries;

		bool m_autoScroll = true;
		bool m_wrap = true;
		bool m_showDebug = true;
		bool m_showInfo = true;
		bool m_showWarning = true;
		bool m_showError = true;
		bool m_showFatal = true;
		bool m_showTodo = true;
		bool m_showFunction = true;
		char m_filter[256]{};
	};
}

