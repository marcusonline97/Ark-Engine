#include "MusicPlayer/MusicPlayer.h"

#if defined(ARK_HAS_FMOD) && ARK_HAS_FMOD

#include <algorithm>
#include <string>

#include <fmod.hpp>

namespace ArkAudio
{
    namespace
    {
        static float Clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

        class FmodMusicBackend final : public IMusicBackend
        {
        public:
            const char* Name() const override { return "FMOD"; }

            bool Init() override
            {
                FMOD_RESULT r = FMOD::System_Create(&m_system);
                if (r != FMOD_OK || !m_system)
                    return false;

                r = m_system->init(32, FMOD_INIT_NORMAL, nullptr);
                if (r != FMOD_OK)
                    return false;

                return true;
            }

            void Shutdown() override
            {
                Stop();
                if (m_system)
                {
                    m_system->release();
                    m_system = nullptr;
                }
            }

            bool PlayFile(const std::filesystem::path& absolutePath) override
            {
                if (!m_system)
                    return false;

                Stop();

                const std::string pathStr = absolutePath.string();
                FMOD_RESULT r = m_system->createStream(pathStr.c_str(), FMOD_DEFAULT, nullptr, &m_sound);
                if (r != FMOD_OK || !m_sound)
                    return false;

                r = m_system->playSound(m_sound, nullptr, false, &m_channel);
                if (r != FMOD_OK || !m_channel)
                    return false;

                m_channel->setVolume(m_volume);
                return true;
            }

            void Stop() override
            {
                if (m_channel)
                {
                    m_channel->stop();
                    m_channel = nullptr;
                }
                if (m_sound)
                {
                    m_sound->release();
                    m_sound = nullptr;
                }
            }

            void Pause(bool paused) override
            {
                if (m_channel)
                    m_channel->setPaused(paused);
            }

            bool IsPlaying() const override
            {
                if (!m_channel)
                    return false;
                bool playing = false;
                m_channel->isPlaying(&playing);
                return playing;
            }

            bool IsPaused() const override
            {
                if (!m_channel)
                    return false;
                bool paused = false;
                m_channel->getPaused(&paused);
                return paused;
            }

            void SetVolume(float volume01) override
            {
                m_volume = Clamp01(volume01);
                if (m_channel)
                    m_channel->setVolume(m_volume);
            }

            void Update() override
            {
                if (m_system)
                    m_system->update();
            }

        private:
            FMOD::System* m_system = nullptr;
            FMOD::Sound* m_sound = nullptr;
            FMOD::Channel* m_channel = nullptr;
            float m_volume = 1.0f;
        };
    }

    std::unique_ptr<IMusicBackend> CreateFmodMusicBackend_Impl()
    {
        return std::make_unique<FmodMusicBackend>();
    }
} 

#endif 