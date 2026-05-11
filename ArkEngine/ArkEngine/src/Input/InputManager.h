#pragma once

#include <array>

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

	private:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------
		std::array <bool, 512> m_keys = {false};
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