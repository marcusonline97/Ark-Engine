#include "Logger.h"
#include <iostream>
#include <atomic>
#include <mutex>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace Logging {

    std::atomic<uint32_t> g_levelMask{ 0 };
    std::atomic<bool>     g_vtInitialized{ false };
    std::atomic<bool>     g_alive{ true };

    std::mutex g_outputMutex;

    constexpr uint32_t Bit(Level level) {
        return 1u << static_cast<uint32_t>(level);
    }

    void EnableLevel(Level level) {
        g_levelMask.fetch_or(Bit(level), std::memory_order_relaxed);
    }

    void DisableLevel(Level level) {
        g_levelMask.fetch_and(~Bit(level), std::memory_order_relaxed);
    }

    bool IsEnabled(Level level) {
        return (g_levelMask.load(std::memory_order_relaxed) & Bit(level)) != 0;
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
        case Level::INIT:     return "\x1b[33m";
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

    MessageStream Message(Level level) {
        return MessageStream(level);
    }

    MessageStream::MessageStream(Level level)
        : m_level(level), m_enabled(IsEnabled(level)) {
    }

    MessageStream::~MessageStream() {
        if (!m_enabled || m_moved) return;
        if (!g_alive.load(std::memory_order_acquire)) return;

        EnableVT();

        const std::string msg = m_ss.str();
        if (msg.empty()) return;

        std::lock_guard<std::mutex> lock(g_outputMutex);

        std::cout
            << Color(m_level)
            << "[" << Name(m_level) << "] "
            << msg
            << "\x1b[0m";

        if (msg.back() != '\n')
            std::cout << '\n';
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
