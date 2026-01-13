#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <vector>

namespace ArkAudio
{
    struct MusicTrack
    {
        std::string name;
        std::filesystem::path path; // absolute path
    };

    struct MusicGenre
    {
        std::string name;
        std::filesystem::path folder; // absolute path
        std::vector<MusicTrack> tracks;
    };

    enum class MusicBackendType
    {
        Null = 0,
        FMOD = 1,
        SoLoud = 2
    };

    struct MusicPlayerConfig
    {
        std::filesystem::path libraryRoot; // e.g. <Project>/ArkEngine/Resources/Music
        MusicBackendType preferredBackend = MusicBackendType::Null;
        bool shuffle = false;
        bool loopPlaylist = true;
        float volume = 1.0f; // 0..1
    };

    class IMusicBackend
    {
    public:
        virtual ~IMusicBackend() = default;
        virtual const char* Name() const = 0;
        virtual bool Init() = 0;
        virtual void Shutdown() = 0;

        virtual bool PlayFile(const std::filesystem::path& absolutePath) = 0;
        virtual void Stop() = 0;
        virtual void Pause(bool paused) = 0;

        virtual bool IsPlaying() const = 0;
        virtual bool IsPaused() const = 0;

        virtual void SetVolume(float volume01) = 0;
        virtual void Update() = 0;
    };

    // Always available. Used when no audio backend is enabled/found.
    std::unique_ptr<IMusicBackend> CreateNullMusicBackend();

    // Optional backends (compiled in only if enabled by build).
    std::unique_ptr<IMusicBackend> CreateFmodMusicBackend();
    std::unique_ptr<IMusicBackend> CreateSoLoudMusicBackend();

    class MusicPlayer
    {
    public:
        MusicPlayer();
        ~MusicPlayer();

        void Init(const MusicPlayerConfig& cfg);
        void Shutdown();
        void Update(); // call every frame

        void SetLibraryRoot(const std::filesystem::path& absoluteRoot);
        const std::filesystem::path& GetLibraryRoot() const { return m_libraryRoot; }
        void RescanLibrary();

        const std::vector<MusicGenre>& GetGenres() const { return m_genres; }
        int GetSelectedGenreIndex() const { return m_selectedGenre; }
        void SetSelectedGenreIndex(int idx);

        int GetSelectedTrackIndex() const { return m_selectedTrack; }
        void SetSelectedTrackIndex(int idx);

        const MusicTrack* GetCurrentTrack() const;

        void SetShuffle(bool enabled);
        bool GetShuffle() const { return m_shuffle; }

        void SetLoopPlaylist(bool enabled) { m_loopPlaylist = enabled; }
        bool GetLoopPlaylist() const { return m_loopPlaylist; }

        void SetVolume(float volume01);
        float GetVolume() const { return m_volume; }

        bool PlaySelected();
        bool PlayTrack(int trackIndex);
        void Stop();
        void TogglePause();
        bool IsPlaying() const;
        bool IsPaused() const;

        void Next();
        void Previous();

        const char* GetBackendName() const;
        MusicBackendType GetBackendType() const { return m_backendType; }

        // Human-readable status string for UI.
        std::string GetStatusText() const;

    private:
        void RebuildPlayOrder();
        int TrackCount() const;
        const MusicGenre* SelectedGenre() const;
        MusicGenre* SelectedGenre();
        bool HasValidSelection() const;

        bool PlayInternalIndex(int internalIndex);
        int ToInternalIndex(int trackIndex) const;
        int FromInternalIndex(int internalIndex) const;

        static bool IsSupportedAudioFile(const std::filesystem::path& p);

    private:
        std::filesystem::path m_libraryRoot;
        std::vector<MusicGenre> m_genres;

        int m_selectedGenre = -1;
        int m_selectedTrack = -1;

        // "Internal index" refers to index into the selected genre's tracks.
        int m_currentInternal = -1;

        bool m_shuffle = false;
        bool m_loopPlaylist = true;
        float m_volume = 1.0f;

        // Play order is a permutation of [0..trackCount-1] when shuffle=true, else identity.
        std::vector<int> m_playOrder;
        int m_playCursor = -1; // index into m_playOrder pointing at currentInternal

        std::mt19937 m_rng;

        MusicBackendType m_backendType = MusicBackendType::Null;
        std::unique_ptr<IMusicBackend> m_backend;

        bool m_initialized = false;
        bool m_userPaused = false;
        bool m_wasPlayingLastFrame = false;
    };
}