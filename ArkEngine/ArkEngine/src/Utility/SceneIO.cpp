#include "SceneIO.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <string_view>

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

	static bool WriteJsonAtomic(const std::filesystem::path& path, const json& root, std::string_view context)
	{
		std::error_code ec;
		std::filesystem::create_directories(path.parent_path(), ec);

		const std::filesystem::path tempPath = path.string() + ".tmp";
		{
			std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);
			if (!out.is_open())
			{
				Logging::Error() << context << ": failed to open temp file '" << tempPath.string() << "'\n";
				return false;
			}

			out << root.dump(2);
			out.flush();
			if (!out.good())
			{
				Logging::Error() << context << ": failed to write JSON to '" << tempPath.string() << "'\n";
				return false;
			}
		}

		ec.clear();
		std::filesystem::rename(tempPath, path, ec);
		if (!ec)
			return true;

		// Cross-volume or rename race fallback.
		std::error_code copyEc;
		std::filesystem::copy_file(
			tempPath,
			path,
			std::filesystem::copy_options::overwrite_existing,
			copyEc);

		std::error_code removeEc;
		std::filesystem::remove(tempPath, removeEc);

		if (copyEc)
		{
			Logging::Error() << context << ": failed to replace '" << path.string()
				<< "': " << copyEc.message() << "\n";
			return false;
		}

		return true;
	}

	static json Vec3ToJson(const glm::vec3& v)
	{
		const auto finiteOrZero = [](float value)
		{
			return std::isfinite(value) ? value : 0.0f;
		};

		return json::array({
			finiteOrZero(v.x),
			finiteOrZero(v.y),
			finiteOrZero(v.z),
			});
	}

	static bool JsonToVec3(const json& j, glm::vec3& out)
	{
		if (!j.is_array() || j.size() != 3)
			return false;

		const auto readFinite = [](const json& value, float& dst)
		{
			if (!value.is_number())
				return false;

			const double asDouble = value.get<double>();
			if (!std::isfinite(asDouble))
				return false;

			dst = static_cast<float>(asDouble);
			return true;
		};

		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		if (!readFinite(j[0], x) || !readFinite(j[1], y) || !readFinite(j[2], z))
			return false;

		out.x = x;
		out.y = y;
		out.z = z;
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

	static void ToJson(json& j, const TextRenderEditorComponent& c)
	{
		j = json{
			{ "displayName", c.displayName },
			{ "text", c.text },
			{ "color", Vec3ToJson(c.color) },
			{ "alpha", c.alpha },
			{ "pixelSize", c.pixelSize },
			{ "centerOnAnchor", c.centerOnAnchor },
			{ "cameraBound", c.cameraBound },
			{ "worldOffset", Vec3ToJson(c.worldOffset) },
			{ "cameraOffset", Vec3ToJson(c.cameraOffset) },
		};
	}

	static void FromJson(const json& j, TextRenderEditorComponent& c)
	{
		c.displayName = j.value("displayName", "Text Render");
		c.text = j.value("text", "New Text");
		(void)JsonToVec3(j.value("color", json::array({ 1.0f, 1.0f, 1.0f })), c.color);
		c.alpha = j.value("alpha", 1.0f);
		c.pixelSize = j.value("pixelSize", 18.0f);
		c.centerOnAnchor = j.value("centerOnAnchor", false);
		c.cameraBound = j.value("cameraBound", false);
		(void)JsonToVec3(j.value("worldOffset", json::array({ 0.0f, 0.0f, 0.0f })), c.worldOffset);
		(void)JsonToVec3(j.value("cameraOffset", json::array({ 0.0f, 0.0f, 2.0f })), c.cameraOffset);
	}

	static void ToJson(json& j, const PhysicsBodyEditorComponent& c)
	{
		j = json{
			{ "displayName", c.displayName },
			{ "motionType", c.motionType },
			{ "useGravity", c.useGravity },
			{ "halfExtents", Vec3ToJson(c.halfExtents) },
		};
	}

	static void FromJson(const json& j, PhysicsBodyEditorComponent& c)
	{
		c.displayName = j.value("displayName", "Physics Body");
		c.motionType = j.value("motionType", 1);
		c.useGravity = j.value("useGravity", true);
		(void)JsonToVec3(j.value("halfExtents", json::array({ 0.5f, 0.5f, 0.5f })), c.halfExtents);
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
		if (o.textRender) { json c; ToJson(c, *o.textRender); j["textRender"] = c; }
		if (o.physicsBody) { json c; ToJson(c, *o.physicsBody); j["physicsBody"] = c; }
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
		o.textRender.reset();
		o.physicsBody.reset();

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

		if (j.contains("textRender") && j["textRender"].is_object())
		{
			TextRenderEditorComponent c{};
			FromJson(j["textRender"], c);
			o.textRender = c;
		}

		if (j.contains("physicsBody") && j["physicsBody"].is_object())
		{
			PhysicsBodyEditorComponent c{};
			FromJson(j["physicsBody"], c);
			o.physicsBody = c;
		}
	}

	bool SaveEditorScene(const std::filesystem::path& path, const std::vector<EditorObject>& objects)
	{
		json root;
		root["version"] = 1;
		root["objects"] = json::array();

		for (const auto& o : objects)
		{
			json j;
			ToJson(j, o);
			root["objects"].push_back(j);
		}

		return WriteJsonAtomic(path, root, "SaveEditorScene");
	}

	bool LoadEditorScene(const std::filesystem::path& path, std::vector<EditorObject>& outObjects)
	{
		std::ifstream in(path, std::ios::binary);
		if (!in.is_open())
		{
			Logging::Error() << "LoadEditorScene: failed to open '" << path.string() << "'\n";
			return false;
		}

		std::string jsonContent(
			std::istreambuf_iterator<char>(in),
			std::istreambuf_iterator<char>());

		if (jsonContent.size() >= 3 &&
			static_cast<unsigned char>(jsonContent[0]) == 0xEF &&
			static_cast<unsigned char>(jsonContent[1]) == 0xBB &&
			static_cast<unsigned char>(jsonContent[2]) == 0xBF)
		{
			jsonContent.erase(0, 3);
		}

		json root;
		std::string parseError;
		try
		{
			root = json::parse(jsonContent);
		}
		catch (const std::exception& e)
		{
			parseError = e.what();
		}

		bool repairedDiscardedTokens = false;
		if (!parseError.empty())
		{
			static constexpr std::string_view kDiscarded = "<discarded>";
			if (jsonContent.find(kDiscarded) != std::string::npos)
			{
				repairedDiscardedTokens = true;
				size_t pos = 0;
				while ((pos = jsonContent.find(kDiscarded, pos)) != std::string::npos)
				{
					jsonContent.replace(pos, kDiscarded.size(), "null");
					pos += 4;
				}

				try
				{
					root = json::parse(jsonContent);
					parseError.clear();
				}
				catch (const std::exception& e)
				{
					parseError = e.what();
				}
			}
		}

		if (!parseError.empty())
		{
			Logging::Error() << "LoadEditorScene: JSON parse error: " << parseError << "\n";
			return false;
		}

		if (repairedDiscardedTokens)
		{
			Logging::Warning() << "LoadEditorScene: repaired invalid '<discarded>' tokens in scene JSON.\n";
			(void)WriteJsonAtomic(path, root, "LoadEditorScene(repair)");
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