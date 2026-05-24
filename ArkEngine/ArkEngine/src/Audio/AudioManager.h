#pragma once

#include "Core/Common.h"
#include <memory>

struct ma_engine;

namespace Engine
{
	class AudioManager
	{
	public:
		AudioManager();
		~AudioManager();

		bool Init();
		ma_engine* GetEngine();

		void SetListenerPosition(const glm::vec3& pos);

	private:
		std::unique_ptr<ma_engine> m_engine;
	};
}