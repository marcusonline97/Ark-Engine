#pragma once

#include <unordered_map>

#include <GLFW/glfw3.h>

#include "Input/KeyCodes.h"

namespace Ark::Input
{
	// Header-only, minimal input wrapper to bind Ark keycodes (KeyCodes.h) to GLFW.
	// Call SetWindow() once, and NewFrame() once per frame (before reading inputs).

	inline GLFWwindow* g_Window = nullptr;
	inline std::unordered_map<int, bool> g_prevKeyDown;
	inline std::unordered_map<int, bool> g_prevMouseDown;

	inline void SetWindow(GLFWwindow* window)
	{
		g_Window = window;
	}

	inline int ToGlfwKey(int arkKey)
	{
		// KeyCodes.h contains a couple of Win32 virtual-key values for left ctrl/shift.
		// Normalize those to the GLFW equivalents when querying GLFW.
		switch (arkKey)
		{
		case ARK_KEY_LEFT_SHIFT:   return ARK_KEY_LEFT_SHIFT_GLFW;
		case ARK_KEY_LEFT_CONTROL: return ARK_KEY_LEFT_CONTROL_GLFW;
		default: break;
		}
		return arkKey;
	}

	inline int ToGlfwMouseButton(int arkMouse)
	{
		switch (arkMouse)
		{
		case ARK_MOUSE_LEFT:  return GLFW_MOUSE_BUTTON_LEFT;
		case ARK_MOUSE_RIGHT: return GLFW_MOUSE_BUTTON_RIGHT;
		default: break;
		}
		return -1;
	}

	inline bool IsKeyDown(int arkKey)
	{
		if (!g_Window) return false;
		const int key = ToGlfwKey(arkKey);
		if (key < 0) return false;
		return glfwGetKey(g_Window, key) == GLFW_PRESS;
	}

	inline bool IsMouseDown(int arkMouse)
	{
		if (!g_Window) return false;
		const int b = ToGlfwMouseButton(arkMouse);
		if (b < 0) return false;
		return glfwGetMouseButton(g_Window, b) == GLFW_PRESS;
	}

	inline void NewFrame()
	{
		// Update previous states for tracked inputs.
		// Note: we intentionally only track keys/buttons we ever queried (data-driven bindings).
		for (auto& kv : g_prevKeyDown)
			kv.second = IsKeyDown(kv.first);
		for (auto& kv : g_prevMouseDown)
			kv.second = IsMouseDown(kv.first);
	}

	inline bool IsKeyPressed(int arkKey)
	{
		const bool down = IsKeyDown(arkKey);
		auto it = g_prevKeyDown.find(arkKey);
		if (it == g_prevKeyDown.end())
		{
			// First time seeing this key: treat as "not pressed last frame".
			g_prevKeyDown.emplace(arkKey, down);
			return down;
		}
		return down && !it->second;
	}

	inline bool IsKeyReleased(int arkKey)
	{
		const bool down = IsKeyDown(arkKey);
		auto it = g_prevKeyDown.find(arkKey);
		if (it == g_prevKeyDown.end())
		{
			g_prevKeyDown.emplace(arkKey, down);
			return false;
		}
		return (!down) && it->second;
	}

	inline bool IsMousePressed(int arkMouse)
	{
		const bool down = IsMouseDown(arkMouse);
		auto it = g_prevMouseDown.find(arkMouse);
		if (it == g_prevMouseDown.end())
		{
			g_prevMouseDown.emplace(arkMouse, down);
			return down;
		}
		return down && !it->second;
	}

	inline bool IsMouseReleased(int arkMouse)
	{
		const bool down = IsMouseDown(arkMouse);
		auto it = g_prevMouseDown.find(arkMouse);
		if (it == g_prevMouseDown.end())
		{
			g_prevMouseDown.emplace(arkMouse, down);
			return false;
		}
		return (!down) && it->second;
	}
}