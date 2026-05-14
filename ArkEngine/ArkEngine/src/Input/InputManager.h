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
		bool IsKeyPressed(int key) const;

		void SetMouseButtonPressed(int button, bool pressed);
		bool IsMouseButtonPressed(int button);

		void SetMousePositionOld(const glm::vec2& pos);
		const glm::vec2& GetMousePositionOld() const;

		void SetMousePositionCurrent(const glm::vec2& pos);
		const glm::vec2& GetMousePositionCurrent() const;


	private:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------
		std::array <bool, 512> m_keys = {false};
		std::array<bool, 16> m_mouseKeys = { false };

		glm::vec2 m_mousePositionOld = glm::vec2(0.0f);
		glm::vec2 m_mousePositionCurrent = glm::vec2(0.0f);

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
}