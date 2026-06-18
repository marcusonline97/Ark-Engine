#include "Logger.h"
#include <iostream>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace Logging {

    std::atomic<uint32_t> g_levelMask{ 0 };
    std::atomic<bool>     g_vtInitialized{ false };
    std::atomic<bool>     g_alive{ true };
    std::chrono::steady_clock::time_point g_startTime = std::chrono::steady_clock::now();

    std::mutex g_outputMutex;

    struct SinkEntry
    {
        SinkId id = 0;
        std::function<void(Level, std::string_view, float)> fn;

    };

	std::mutex g_sinksMutex;
	std::vector<SinkEntry> g_sinks;
	std::atomic<SinkId> g_nextSinkId{ 1 };


    constexpr uint32_t Bit(Level level) {
        return 1u << static_cast<uint32_t>(level);
    }

    static void DispatchToSinks(Level level, std::string_view msg, float timestamp)
    {
        std::vector<SinkEntry> sinksCopy;
        {
            std::lock_guard<std::mutex> lock(g_sinksMutex);
            sinksCopy = g_sinks;
		}

        for (const auto& sink : sinksCopy)
        {
			if (sink.fn) sink.fn(level, msg, timestamp);
		}
    }

    SinkId AddSink(std::function<void(Level, std::string_view, float)> sink) {
        if (!sink) return 0;
        const SinkId id = g_nextSinkId.fetch_add(1, std::memory_order_relaxed);
        {
            std::lock_guard<std::mutex> lock(g_sinksMutex);
            g_sinks.push_back(SinkEntry{ id, std::move(sink) });
        }
        return id;
    }

    void RemoveSink(SinkId id) {
        if (id == 0) return;
        std::lock_guard<std::mutex> lock(g_sinksMutex);
        for (size_t i = 0; i < g_sinks.size(); ++i) {
            if (g_sinks[i].id == id) {
                g_sinks.erase(g_sinks.begin() + static_cast<std::ptrdiff_t>(i));
                return;
            }
        }
    }

    void SetStartTime()
    {
        g_startTime = std::chrono::steady_clock::now();
    }

    float GetElapsedSeconds()
    {
        return std::chrono::duration<float>(std::chrono::steady_clock::now() - g_startTime).count();
    }


    void EnableLevel(Level level) {
        g_levelMask.fetch_or(Bit(level), std::memory_order_relaxed);
    }

    void DisableLevel(Level level) {
        g_levelMask.fetch_and(~Bit(level), std::memory_order_relaxed);
    }

    bool IsLevelEnabled(Level level)
    {
        return (g_levelMask.load(std::memory_order_relaxed) & Bit(level)) != 0;
    }

    void EnableAllLevels()
    {
        constexpr uint32_t all = 
            Bit(Level::INIT) |
			Bit(Level::_ERROR) |
			Bit(Level::WARNING) |
			Bit(Level::DEBUG) |
			Bit(Level::FATAL) |
			Bit(Level::TODO) |
            Bit(Level::FUNCTION);
		g_levelMask.store(all, std::memory_order_relaxed);
    }

    void DisableAllLevels()
    {
        g_levelMask.store(0, std::memory_order_relaxed);
    }

#if defined(_WIN32)
    void EnableVT() {
        bool expected = false;
        if (!g_vtInitialized.compare_exchange_strong(expected, true))
            return;

        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        if (h == INVALID_HANDLE_VALUE) return;

        DWORD mode = 0;
        if (!GetConsoleMode(h, &mode)) return;

        SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#else
    void EnableVT() {}
#endif

    const char* Color(Level level) {
        switch (level) {
        case Level::INIT:     return "\x1b[35m";
        case Level::DEBUG:    return "\x1b[36m";
        case Level::_ERROR:    return "\x1b[38;5;160m";
        case Level::WARNING:  return "\x1b[33m";
        case Level::FATAL:    return "\x1b[38;5;196m";
        case Level::TODO:     return "\x1b[38;5;46m";
        case Level::FUNCTION: return "\x1b[38;5;39m";
        default:              return "\x1b[97m";
        }
    }

    const char* Name(Level level) {
        switch (level) {
        case Level::INIT: return "INIT";
        case Level::DEBUG: return "DEBUG";
        case Level::_ERROR: return "ERROR";
        case Level::WARNING: return "WARNING";
        case Level::FATAL: return "FATAL";
        case Level::TODO: return "TODO";
        case Level::FUNCTION: return "FUNCTION";
        }
        return "UNKNOWN";
    }

    const char* LevelName(Level level)
    {
        return Name(level);
    }

    MessageStream Message(Level level) {
        return MessageStream(level);
    }

    MessageStream::MessageStream(Level level)
        : m_level(level), m_enabled(IsLevelEnabled(level)) {
    }

    MessageStream::~MessageStream() {
        if (!m_enabled || m_moved) return;
        if (!g_alive.load(std::memory_order_acquire)) return;

        EnableVT();

        const std::string msg = m_ss.str();
        if (msg.empty()) return;
        const float ts = GetElapsedSeconds();
        {
            std::lock_guard<std::mutex> lock(g_outputMutex);

            std::cout << Color(m_level) << "[" << std::fixed << std::setprecision(3) << ts << "s]"
                << "[" << Name(m_level) << "] "
                << msg
				<< "\x1b[0m";

            if (msg.back() != '\n')
				std::cout << '\n';
        }

		DispatchToSinks(m_level, msg, ts); // races with other threads in the console output, but that's generally fine for logging
    }

    MessageStream::MessageStream(MessageStream&& rhs) noexcept
        : m_level(rhs.m_level),
        m_enabled(rhs.m_enabled),
        m_moved(false),
        m_ss(std::move(rhs.m_ss)) {
        rhs.m_moved = true;
    }

    MessageStream& MessageStream::operator=(MessageStream&& rhs) noexcept {
        if (this == &rhs) return *this;
        m_level = rhs.m_level;
        m_enabled = rhs.m_enabled;
        m_ss = std::move(rhs.m_ss);
        m_moved = false;
        rhs.m_moved = true;
        return *this;
    }

    void Shutdown() {
        g_alive.store(false, std::memory_order_release);
    }
}
