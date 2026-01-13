#include "Audio/MusicPlayer.h"

#if defined(ARK_HAS_SOLOUD) && ARK_HAS_SOLOUD

// Note: This backend is intentionally minimal. It is compiled only when
// ARK_HAS_SOLOUD is enabled by the build system and SoLoud headers/libs exist.

#include <algorithm>
#include <string>

// SoLoud headers (expected on include path when enabled)
#include <soloud.h>
#include <soloud_wavstream.h>

namespace ArkAudio
{
    namespace
    {
        static float Clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

        class SoLoudMusicBackend final : public IMusicBackend
        {
        public:
            const char* Name() const override { return "SoLoud"; }

            bool Init() override
            {
                const auto r = m_soloud.init();
                return r == 0;
            }

            void Shutdown() override
            {
                Stop();
                m_soloud.deinit();
            }

            bool PlayFile(const std::filesystem::path& absolutePath) override
            {
                Stop();

                const std::string p = absolutePath.string();
                if (m_stream.load(p.c_str()) != 0)
                    return false;

                m_handle = m_soloud.play(m_stream);
                m_soloud.setVolume(m_handle, m_volume);
                return m_handle != 0;
            }

            void Stop() override
            {
                if (m_handle != 0)
                {
                    m_soloud.stop(m_handle);
                    m_handle = 0;
                }
            }

            void Pause(bool paused) override
            {
                if (m_handle != 0)
                    m_soloud.setPause(m_handle, paused ? 1 : 0);
            }

            bool IsPlaying() const override
            {
                if (m_handle == 0) return false;
                return m_soloud.isValidVoiceHandle(m_handle) != 0;
            }

            bool IsPaused() const override
            {
                if (m_handle == 0) return false;
                return m_soloud.getPause(m_handle) != 0;
            }

            void SetVolume(float volume01) override
            {
                m_volume = Clamp01(volume01);
                if (m_handle != 0)
                    m_soloud.setVolume(m_handle, m_volume);
            }

            void Update() override {}

        private:
            SoLoud::Soloud m_soloud;
            SoLoud::WavStream m_stream;
            SoLoud::handle m_handle = 0;
            float m_volume = 1.0f;
        };
    } // namespace

    std::unique_ptr<IMusicBackend> CreateSoLoudMusicBackend_Impl()
    {
        return std::make_unique<SoLoudMusicBackend>();
    }
} // namespace ArkAudio

#endif // ARK_HAS_SOLOUD

