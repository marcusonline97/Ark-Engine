#pragma once
#include <string>
#include <sstream>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <string_view>

namespace Logging {

    enum struct Level {
        INIT,
        _ERROR,
        WARNING,
        DEBUG,
        FATAL,
        TODO,
        FUNCTION
    };

    using SinkId = uint32_t;
	SinkId AddSink(std::function<void(Level, std::string_view)> sink);
	void RemoveSink(SinkId id);

    struct MessageStream {
        explicit MessageStream(Level level);
        ~MessageStream();

        MessageStream(const MessageStream&) = delete;
        MessageStream& operator=(const MessageStream&) = delete;

        MessageStream(MessageStream&&) noexcept;
        MessageStream& operator=(MessageStream&&) noexcept;

        template<class T, std::enable_if_t<!std::is_enum_v<T>, int> = 0>
        MessageStream& operator<<(const T& v) {
            if (m_enabled) m_ss << v;
            return *this;
        }

        template<class E, std::enable_if_t<std::is_enum_v<E>, int> = 0>
        MessageStream& operator<<(E e) {
            if (m_enabled)
                m_ss << static_cast<std::underlying_type_t<E>>(e);
            return *this;
        }

    private:
        Level m_level;
        bool  m_enabled = false;
        bool  m_moved = false;
        std::ostringstream m_ss;
    };

    void EnableLevel(Level level);
    void DisableLevel(Level level);
    bool IsEnabled(Level level);

    MessageStream Message(Level level);

    inline MessageStream Init() { return Message(Level::INIT); }
    inline MessageStream Debug() { return Message(Level::DEBUG); }
    inline MessageStream Warning() { return Message(Level::WARNING); }
    inline MessageStream Error() { return Message(Level::_ERROR); }
    inline MessageStream Fatal() { return Message(Level::FATAL); }
    inline MessageStream ToDo() { return Message(Level::TODO); }
    inline MessageStream Function() { return Message(Level::FUNCTION); }

    // MUST be called at shutdown
    void Shutdown();
}
