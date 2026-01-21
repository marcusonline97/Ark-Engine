#include "SceneIO.h"

#include <fstream>

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "Editor/EditorUI.h"
#include "Logger.h"

namespace Ark::Editor
{
	using nlohmann::json;

	static std::string NormalizePathString(std::string s)
	{
		// normalize slashes for JSON stability
		std::replace(s.begin(), s.end(), '\\', '/');
		return s;
	}

	static std::string MakeProjectRelativePathString(const std::filesystem::path& p)
	{
		if (p.empty())
			return {};

		std::error_code ec;
		const auto cwd = std::filesystem::current_path(ec);
		if (ec)
			return NormalizePathString(p.string());

		const auto abs = std::filesystem::absolute(p, ec).lexically_normal();
		if (ec)
			return NormalizePathString(p.string());

		const auto rel = std::filesystem::relative(abs, cwd, ec);
		if (ec)
			return NormalizePathString(abs.string());

		return NormalizePathString(rel.string());
	}

	static std::string SanitizeAssetPath(std::string s)
	{
		s = NormalizePathString(std::move(s));
		if (s.empty())
			return {};

		// If user accidentally saved absolute paths, try to make them project-relative.
		std::error_code ec;
		const auto p = std::filesystem::path(s);
		if (p.is_absolute())
			return MakeProjectRelativePathString(p);

		return s;
	}

	static json Vec3ToJson(const glm::vec3& v)
	{
		return json::array({ v.x, v.y, v.z });
	}

	static bool JsonToVec3(const json& j, glm::vec3& out)
	{
		if (!j.is_array() || j.size() != 3)
			return false;

		out.x = j[0].get<float>();
		out.y = j[1].get<float>();
		out.z = j[2].get<float>();
		return true;
	}

	static void ToJson(json& j, const StaticMeshEditorComponent& c)
	{
		j = json{
			{ "meshPath", SanitizeAssetPath(c.meshPath) },
			{ "displayName", c.displayName },
			{ "materialPath", SanitizeAssetPath(c.materialPath) },
			{ "texturePath", SanitizeAssetPath(c.texturePath) },
		};

		j["textures"] = json::array();
		for (const auto& t : c.textures)
		{
			j["textures"].push_back(json{
				{ "name", t.name },
				{ "path", SanitizeAssetPath(t.path) },
			});
		}
	}

	static void FromJson(const json& j, StaticMeshEditorComponent& c)
	{
		c.meshPath = SanitizeAssetPath(j.value("meshPath", ""));
		c.displayName = j.value("displayName", "Static Mesh");
		c.materialPath = SanitizeAssetPath(j.value("materialPath", ""));
		c.texturePath = SanitizeAssetPath(j.value("texturePath", ""));

		c.textures.clear();
		if (j.contains("textures") && j["textures"].is_array())
		{
			for (const auto& it : j["textures"])
			{
				if (!it.is_object())
					continue;

				StaticMeshEditorComponent::TextureSlot slot{};
				slot.name = it.value("name", "");
				slot.path = SanitizeAssetPath(it.value("path", ""));
				if (!slot.name.empty() || !slot.path.empty())
					c.textures.push_back(std::move(slot));
			}
		}
	}

	static void ToJson(json& j, const SkeletalMeshEditorComponent& c)
	{
		j = json{
			{ "meshPath", SanitizeAssetPath(c.meshPath) },
			{ "displayName", c.displayName },
			{ "animationPath", SanitizeAssetPath(c.animationPath) },
			{ "animationIndex", c.animationIndex },
			{ "materialPath", SanitizeAssetPath(c.materialPath) },
			{ "texturePath", SanitizeAssetPath(c.texturePath) },
		};
	}

	static void FromJson(const json& j, SkeletalMeshEditorComponent& c)
	{
		c.meshPath = SanitizeAssetPath(j.value("meshPath", ""));
		c.displayName = j.value("displayName", "Skeletal Mesh");
		c.animationPath = SanitizeAssetPath(j.value("animationPath", ""));
		c.animationIndex = j.value("animationIndex", -1);
		c.materialPath = SanitizeAssetPath(j.value("materialPath", ""));
		c.texturePath = SanitizeAssetPath(j.value("texturePath", ""));
	}

	// IMPORTANT: these MUST be declared before ToJson(EditorObject) / FromJson(EditorObject)
	static void ToJson(json& j, const CameraEditorComponent& c)
	{
		j = json{
			{ "displayName", c.displayName },
			{ "primary", c.primary },
			{ "fovDeg", c.fovDeg },
			{ "nearPlane", c.nearPlane },
			{ "farPlane", c.farPlane },
		};
	}

	static void FromJson(const json& j, CameraEditorComponent& c)
	{
		c.displayName = j.value("displayName", "Camera");
		c.primary = j.value("primary", true);
		c.fovDeg = j.value("fovDeg", 45.0f);
		c.nearPlane = j.value("nearPlane", 0.1f);
		c.farPlane = j.value("farPlane", 100.0f);
	}

	static void ToJson(json& j, const PointLightEditorComponent& c)
	{
		j = json{
			{ "displayName", c.displayName },
			{ "color", Vec3ToJson(c.color) },
			{ "intensity", c.intensity },
			{ "radius", c.radius },
		};
	}

	static void FromJson(const json& j, PointLightEditorComponent& c)
	{
		c.displayName = j.value("displayName", "Point Light");
		(void)JsonToVec3(j.value("color", json::array({ 1.0f, 1.0f, 1.0f })), c.color);
		c.intensity = j.value("intensity", 1.0f);
		c.radius = j.value("radius", 1.0f);
	}

	static void ToJson(json& j, const EditorObject& o)
	{
		j = json{
			{ "id", o.id },
			{ "parentId", o.parentId },
			{ "name", o.name },
			{ "enabled", o.enabled },
			{ "position", Vec3ToJson(o.position) },
			{ "rotationDeg", Vec3ToJson(o.rotationDeg) },
			{ "scale", Vec3ToJson(o.scale) },
			{ "tint", Vec3ToJson(o.tint) },
			{ "materialPreset", o.materialPreset },
			{ "flipUvV", o.flipUvV },
		};

		if (o.staticMesh) { json c; ToJson(c, *o.staticMesh); j["staticMesh"] = c; }
		if (o.skeletalMesh) { json c; ToJson(c, *o.skeletalMesh); j["skeletalMesh"] = c; }
		if (o.camera) { json c; ToJson(c, *o.camera); j["camera"] = c; }
		if (o.pointLight) { json c; ToJson(c, *o.pointLight); j["pointLight"] = c; }
	}

	static void FromJson(const json& j, EditorObject& o)
	{
		o.id = j.value("id", 0u);
		o.parentId = j.value("parentId", 0u);
		o.name = j.value("name", "GameObject");
		o.enabled = j.value("enabled", true);

		(void)JsonToVec3(j.value("position", json::array({ 0.0f, 0.0f, 0.0f })), o.position);
		(void)JsonToVec3(j.value("rotationDeg", json::array({ 0.0f, 0.0f, 0.0f })), o.rotationDeg);
		(void)JsonToVec3(j.value("scale", json::array({ 1.0f, 1.0f, 1.0f })), o.scale);
		(void)JsonToVec3(j.value("tint", json::array({ 1.0f, 1.0f, 1.0f })), o.tint);

		o.materialPreset = j.value("materialPreset", 0);
		o.flipUvV = j.value("flipUvV", false);

		o.staticMesh.reset();
		o.skeletalMesh.reset();
		o.camera.reset();
		o.pointLight.reset();

		if (j.contains("staticMesh") && j["staticMesh"].is_object())
		{
			StaticMeshEditorComponent c{};
			FromJson(j["staticMesh"], c);
			o.staticMesh = c;
		}

		if (j.contains("skeletalMesh") && j["skeletalMesh"].is_object())
		{
			SkeletalMeshEditorComponent c{};
			FromJson(j["skeletalMesh"], c);
			o.skeletalMesh = c;
		}

		if (j.contains("camera") && j["camera"].is_object())
		{
			CameraEditorComponent c{};
			FromJson(j["camera"], c);
			o.camera = c;
		}

		if (j.contains("pointLight") && j["pointLight"].is_object())
		{
			PointLightEditorComponent c{};
			FromJson(j["pointLight"], c);
			o.pointLight = c;
		}
	}

	bool SaveEditorScene(const std::filesystem::path& path, const std::vector<EditorObject>& objects)
	{
		std::error_code ec;
		std::filesystem::create_directories(path.parent_path(), ec);

		std::ofstream out(path, std::ios::binary);
		if (!out.is_open())
		{
			Logging::Error() << "SaveEditorScene: failed to open '" << path.string() << "'\n";
			return false;
		}

		json root;
		root["version"] = 1;
		root["objects"] = json::array();

		for (const auto& o : objects)
		{
			json j;
			ToJson(j, o);
			root["objects"].push_back(j);
		}

		out << root.dump(2);
		return true;
	}

	bool LoadEditorScene(const std::filesystem::path& path, std::vector<EditorObject>& outObjects)
	{
		std::ifstream in(path, std::ios::binary);
		if (!in.is_open())
		{
			Logging::Error() << "LoadEditorScene: failed to open '" << path.string() << "'\n";
			return false;
		}

		json root;
		try
		{
			in >> root;
		}
		catch (const std::exception& e)
		{
			Logging::Error() << "LoadEditorScene: JSON parse error: " << e.what() << "\n";
			return false;
		}

		if (!root.contains("objects") || !root["objects"].is_array())
		{
			Logging::Error() << "LoadEditorScene: invalid file (missing 'objects' array)\n";
			return false;
		}

		std::vector<EditorObject> loaded;
		for (const auto& j : root["objects"])
		{
			if (!j.is_object())
				continue;

			EditorObject o{};
			FromJson(j, o);
			loaded.push_back(std::move(o));
		}

		outObjects = std::move(loaded);
		return true;
	}
}