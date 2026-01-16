#pragma once

#include <filesystem>
#include <vector>

struct EditorObject;

namespace Ark::Editor
{
	bool SaveEditorScene(const std::filesystem::path& path, const std::vector<EditorObject>& objects);
	bool LoadEditorScene(const std::filesystem::path& path, std::vector<EditorObject>& outObjects);
}