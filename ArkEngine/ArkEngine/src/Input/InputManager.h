#pragma once

#include <array>
#include <glm/vec2.hpp>


namespace Engine
{
	class InputManager
	{
	public:

		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------


		//-------------------------------------------
		// Functions
		//-------------------------------------------
		void SetKeyPressed(int key, bool pressed);
		bool IsKeyPressed(int key);

		void SetMouseButtonPressed(int button, bool pressed);
		bool IsMouseButtonPressed(int button);

		void SetMousePositionOld(const glm::vec2& pos);
		const glm::vec2& GetMousePositionOld() const;

		void SetMousePositionCurrent(const glm::vec2& pos);
		const glm::vec2& GetMousePositionCurrent() const;

		void SetMousePositionChanged(bool changed);
		bool IsMousePositionChanged() const;

	private:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------
		std::array <bool, 512> m_keys = {false};
		std::array<bool, 16> m_mouseKeys = { false };

		glm::vec2 m_mousePositionOld = glm::vec2(0.0f);
		glm::vec2 m_mousePositionCurrent = glm::vec2(0.0f);
		bool m_mousePositionChanged = false;
		friend class ArkEngine;

		//-------------------------------------------
		// Functions
		//-------------------------------------------
		InputManager() = default;
		InputManager(const InputManager&) = delete;
		InputManager(InputManager&&) = delete;
		InputManager& operator=(const InputManager&) = delete;
		InputManager& operator=(InputManager&&) = delete;

	};

	// might look into implementing the keys as an enum class using uint16_t later on to group them into categories and make it more readable, but for now this is fine
}