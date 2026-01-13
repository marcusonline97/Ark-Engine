#include "Audio/MusicPlayer.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <sstream>

namespace ArkAudio
{
    namespace
    {
        static std::string ToLower(std::string s)
        {
            for (char& c : s)
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            return s;
        }

        class NullMusicBackend final : public IMusicBackend
        {
        public:
            const char* Name() const override { return "Null"; }
            bool Init() override { m_inited = true; return true; }
            void Shutdown() override { m_inited = false; m_playing = false; m_paused = false; m_current.clear(); }

            bool PlayFile(const std::filesystem::path& absolutePath) override
            {
                if (!m_inited) return false;
                m_current = absolutePath;
                m_playing = true;
                m_paused = false;
                return true;
            }

            void Stop() override { m_playing = false; m_paused = false; }
            void Pause(bool paused) override { if (m_playing) m_paused = paused; }

            bool IsPlaying() const override { return m_playing; }
            bool IsPaused() const override { return m_paused; }

            void SetVolume(float volume01) override { m_volume = volume01; }
            void Update() override {}

        private:
            bool m_inited = false;
            bool m_playing = false;
            bool m_paused = false;
            float m_volume = 1.0f;
            std::filesystem::path m_current;
        };
    } // namespace

    std::unique_ptr<IMusicBackend> CreateNullMusicBackend()
    {
        return std::make_unique<NullMusicBackend>();
    }

    // Build-time optional backends. If not enabled, these return nullptr.
    std::unique_ptr<IMusicBackend> CreateFmodMusicBackend()
    {
#if defined(ARK_HAS_FMOD) && ARK_HAS_FMOD
        // Implemented in a separate TU when ARK_HAS_FMOD is enabled.
        extern std::unique_ptr<IMusicBackend> CreateFmodMusicBackend_Impl();
        return CreateFmodMusicBackend_Impl();
#else
        return nullptr;
#endif
    }

    std::unique_ptr<IMusicBackend> CreateSoLoudMusicBackend()
    {
#if defined(ARK_HAS_SOLOUD) && ARK_HAS_SOLOUD
        extern std::unique_ptr<IMusicBackend> CreateSoLoudMusicBackend_Impl();
        return CreateSoLoudMusicBackend_Impl();
#else
        return nullptr;
#endif
    }

    MusicPlayer::MusicPlayer()
        : m_rng(static_cast<uint32_t>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count()))
    {
    }

    MusicPlayer::~MusicPlayer()
    {
        Shutdown();
    }

    void MusicPlayer::Init(const MusicPlayerConfig& cfg)
    {
        Shutdown();

        m_libraryRoot = cfg.libraryRoot;
        m_shuffle = cfg.shuffle;
        m_loopPlaylist = cfg.loopPlaylist;
        m_volume = std::clamp(cfg.volume, 0.0f, 1.0f);

        // Backend selection: try preferred, then fall back to any compiled backend, else Null.
        m_backendType = MusicBackendType::Null;
        m_backend = nullptr;

        auto pickBackend = [&](MusicBackendType t) -> bool
        {
            std::unique_ptr<IMusicBackend> candidate;
            switch (t)
            {
            case MusicBackendType::FMOD: candidate = CreateFmodMusicBackend(); break;
            case MusicBackendType::SoLoud: candidate = CreateSoLoudMusicBackend(); break;
            case MusicBackendType::Null: candidate = CreateNullMusicBackend(); break;
            }
            if (!candidate) return false;
            if (!candidate->Init()) return false;
            m_backendType = t;
            m_backend = std::move(candidate);
            m_backend->SetVolume(m_volume);
            return true;
        };

        // Preferred first.
        if (!pickBackend(cfg.preferredBackend))
        {
            // Then any compiled-in real backend.
            if (!pickBackend(MusicBackendType::FMOD))
                if (!pickBackend(MusicBackendType::SoLoud))
                    pickBackend(MusicBackendType::Null);
        }

        RescanLibrary();
        m_initialized = true;
    }

    void MusicPlayer::Shutdown()
    {
        if (m_backend)
        {
            m_backend->Shutdown();
            m_backend.reset();
        }

        m_genres.clear();
        m_libraryRoot.clear();
        m_selectedGenre = -1;
        m_selectedTrack = -1;
        m_currentInternal = -1;
        m_playOrder.clear();
        m_playCursor = -1;
        m_initialized = false;
        m_userPaused = false;
        m_wasPlayingLastFrame = false;
    }

    void MusicPlayer::Update()
    {
        if (!m_initialized || !m_backend) return;

        m_backend->Update();

        const bool playing = m_backend->IsPlaying();
        const bool paused = m_backend->IsPaused();

        // Auto-advance when playback ends (only if we weren't paused by user).
        if (m_wasPlayingLastFrame && !playing && !paused && !m_userPaused)
        {
            Next();
        }

        m_wasPlayingLastFrame = playing;
    }

    void MusicPlayer::SetLibraryRoot(const std::filesystem::path& absoluteRoot)
    {
        m_libraryRoot = absoluteRoot;
    }

    bool MusicPlayer::IsSupportedAudioFile(const std::filesystem::path& p)
    {
        if (!p.has_extension()) return false;
        const std::string ext = ToLower(p.extension().string());
        return ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac" || ext == ".aiff" || ext == ".aif";
    }

    void MusicPlayer::RescanLibrary()
    {
        m_genres.clear();
        m_selectedGenre = -1;
        m_selectedTrack = -1;
        m_currentInternal = -1;
        m_playOrder.clear();
        m_playCursor = -1;

        std::error_code ec;
        if (m_libraryRoot.empty() || !std::filesystem::exists(m_libraryRoot, ec))
            return;

        for (const auto& dirEntry : std::filesystem::directory_iterator(m_libraryRoot, ec))
        {
            if (ec) break;
            if (!dirEntry.is_directory(ec)) continue;

            MusicGenre genre;
            genre.folder = dirEntry.path();
            genre.name = dirEntry.path().filename().string();

            std::error_code ec2;
            for (const auto& fileEntry : std::filesystem::directory_iterator(genre.folder, ec2))
            {
                if (ec2) break;
                if (!fileEntry.is_regular_file(ec2)) continue;
                const auto& p = fileEntry.path();
                if (!IsSupportedAudioFile(p)) continue;

                MusicTrack t;
                t.path = std::filesystem::absolute(p, ec2);
                t.name = p.stem().string();
                genre.tracks.push_back(std::move(t));
            }

            std::sort(genre.tracks.begin(), genre.tracks.end(),
                [](const MusicTrack& a, const MusicTrack& b)
                {
                    return a.name < b.name;
                });

            if (!genre.tracks.empty())
                m_genres.push_back(std::move(genre));
        }

        std::sort(m_genres.begin(), m_genres.end(),
            [](const MusicGenre& a, const MusicGenre& b)
            {
                return a.name < b.name;
            });

        if (!m_genres.empty())
        {
            m_selectedGenre = 0;
            m_selectedTrack = 0;
            RebuildPlayOrder();
        }
    }

    int MusicPlayer::TrackCount() const
    {
        const MusicGenre* g = SelectedGenre();
        return g ? static_cast<int>(g->tracks.size()) : 0;
    }

    const MusicGenre* MusicPlayer::SelectedGenre() const
    {
        if (m_selectedGenre < 0 || m_selectedGenre >= static_cast<int>(m_genres.size()))
            return nullptr;
        return &m_genres[m_selectedGenre];
    }

    MusicGenre* MusicPlayer::SelectedGenre()
    {
        if (m_selectedGenre < 0 || m_selectedGenre >= static_cast<int>(m_genres.size()))
            return nullptr;
        return &m_genres[m_selectedGenre];
    }

    bool MusicPlayer::HasValidSelection() const
    {
        const int count = TrackCount();
        return count > 0 && m_selectedTrack >= 0 && m_selectedTrack < count;
    }

    void MusicPlayer::SetSelectedGenreIndex(int idx)
    {
        if (idx < 0 || idx >= static_cast<int>(m_genres.size()))
            return;
        if (m_selectedGenre == idx)
            return;

        Stop();
        m_selectedGenre = idx;
        m_selectedTrack = TrackCount() > 0 ? 0 : -1;
        m_currentInternal = -1;
        RebuildPlayOrder();
    }

    void MusicPlayer::SetSelectedTrackIndex(int idx)
    {
        const int count = TrackCount();
        if (idx < 0 || idx >= count)
            return;
        m_selectedTrack = idx;
    }

    void MusicPlayer::SetShuffle(bool enabled)
    {
        if (m_shuffle == enabled)
            return;

        m_shuffle = enabled;
        RebuildPlayOrder();
    }

    void MusicPlayer::SetVolume(float volume01)
    {
        m_volume = std::clamp(volume01, 0.0f, 1.0f);
        if (m_backend)
            m_backend->SetVolume(m_volume);
    }

    void MusicPlayer::RebuildPlayOrder()
    {
        m_playOrder.clear();
        m_playCursor = -1;

        const int count = TrackCount();
        if (count <= 0)
            return;

        m_playOrder.resize(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) m_playOrder[static_cast<size_t>(i)] = i;

        if (m_shuffle)
            std::shuffle(m_playOrder.begin(), m_playOrder.end(), m_rng);

        // Attempt to keep cursor aligned with current track if possible.
        if (m_currentInternal >= 0 && m_currentInternal < count)
        {
            for (int i = 0; i < count; ++i)
            {
                if (m_playOrder[static_cast<size_t>(i)] == m_currentInternal)
                {
                    m_playCursor = i;
                    break;
                }
            }
        }
    }

    int MusicPlayer::ToInternalIndex(int trackIndex) const
    {
        // trackIndex is already internal (selected-genre local index)
        return trackIndex;
    }

    int MusicPlayer::FromInternalIndex(int internalIndex) const
    {
        return internalIndex;
    }

    bool MusicPlayer::PlayInternalIndex(int internalIndex)
    {
        if (!m_backend) return false;

        MusicGenre* g = SelectedGenre();
        if (!g) return false;
        if (internalIndex < 0 || internalIndex >= static_cast<int>(g->tracks.size()))
            return false;

        const auto& t = g->tracks[static_cast<size_t>(internalIndex)];
        m_userPaused = false;

        if (!m_backend->PlayFile(t.path))
            return false;

        m_currentInternal = internalIndex;
        m_selectedTrack = FromInternalIndex(internalIndex);

        // Update cursor to match.
        for (int i = 0; i < static_cast<int>(m_playOrder.size()); ++i)
        {
            if (m_playOrder[static_cast<size_t>(i)] == internalIndex)
            {
                m_playCursor = i;
                break;
            }
        }

        m_wasPlayingLastFrame = true;
        return true;
    }

    bool MusicPlayer::PlaySelected()
    {
        if (!HasValidSelection())
            return false;
        return PlayTrack(m_selectedTrack);
    }

    bool MusicPlayer::PlayTrack(int trackIndex)
    {
        const int internal = ToInternalIndex(trackIndex);
        return PlayInternalIndex(internal);
    }

    void MusicPlayer::Stop()
    {
        if (m_backend)
            m_backend->Stop();
        m_userPaused = false;
        m_wasPlayingLastFrame = false;
    }

    void MusicPlayer::TogglePause()
    {
        if (!m_backend) return;
        if (!m_backend->IsPlaying()) return;
        const bool paused = m_backend->IsPaused();
        m_backend->Pause(!paused);
        m_userPaused = !paused;
    }

    bool MusicPlayer::IsPlaying() const
    {
        return m_backend ? m_backend->IsPlaying() : false;
    }

    bool MusicPlayer::IsPaused() const
    {
        return m_backend ? m_backend->IsPaused() : false;
    }

    const MusicTrack* MusicPlayer::GetCurrentTrack() const
    {
        const MusicGenre* g = SelectedGenre();
        if (!g) return nullptr;
        if (m_currentInternal < 0 || m_currentInternal >= static_cast<int>(g->tracks.size()))
            return nullptr;
        return &g->tracks[static_cast<size_t>(m_currentInternal)];
    }

    void MusicPlayer::Next()
    {
        const int count = TrackCount();
        if (count <= 0) return;
        if (m_playOrder.empty()) RebuildPlayOrder();
        if (m_playOrder.empty()) return;

        int nextCursor = m_playCursor;
        if (nextCursor < 0) nextCursor = 0;
        else nextCursor += 1;

        if (nextCursor >= static_cast<int>(m_playOrder.size()))
        {
            if (!m_loopPlaylist)
            {
                Stop();
                return;
            }
            nextCursor = 0;
        }

        m_playCursor = nextCursor;
        const int nextInternal = m_playOrder[static_cast<size_t>(m_playCursor)];
        PlayInternalIndex(nextInternal);
    }

    void MusicPlayer::Previous()
    {
        const int count = TrackCount();
        if (count <= 0) return;
        if (m_playOrder.empty()) RebuildPlayOrder();
        if (m_playOrder.empty()) return;

        int prevCursor = m_playCursor;
        if (prevCursor < 0) prevCursor = 0;
        else prevCursor -= 1;

        if (prevCursor < 0)
        {
            if (!m_loopPlaylist)
            {
                Stop();
                return;
            }
            prevCursor = static_cast<int>(m_playOrder.size()) - 1;
        }

        m_playCursor = prevCursor;
        const int prevInternal = m_playOrder[static_cast<size_t>(m_playCursor)];
        PlayInternalIndex(prevInternal);
    }

    const char* MusicPlayer::GetBackendName() const
    {
        return m_backend ? m_backend->Name() : "None";
    }

    std::string MusicPlayer::GetStatusText() const
    {
        std::ostringstream ss;
        ss << "Backend: " << GetBackendName();

        const MusicTrack* cur = GetCurrentTrack();
        if (cur)
            ss << " | Now: " << cur->name;
        else
            ss << " | Now: (none)";

        if (IsPlaying())
        {
            ss << (IsPaused() ? " [Paused]" : " [Playing]");
        }
        else
        {
            ss << " [Stopped]";
        }

        return ss.str();
    }
} // namespace ArkAudio

