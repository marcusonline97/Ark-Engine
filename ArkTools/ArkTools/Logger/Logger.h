#pragma once 

#include <string>
#include <string_view>
#include <source_location>
#include <vector>
#include <cassert>
#include <mutex>

/*
General Purpose Logger for Common Purpose Error Handling In Console & Editor
*/

#define ARK_LOG(x, ...) ARK_LOGGER::Logger::GetInstance().Log(x, __VA_ARGS__);
#define ARK_WARN(x, ...) ARK_LOGGER::Logger::GetInstance().Warn(x, __VA_ARGS__);
#define ARK_ERROR(x, ...) ARK_LOGGER::Logger::GetInstance().Error(std::source_location::current(), x, __VA_ARGS__);

#define ARK_INIT_LOGS(console, retain) ARK_LOGGER::Logger::GetInstance().Init(console, retain);

namespace ARK_LOGGER
{
	struct LogEntry
	{
		enum class LogType
		{
			INFO,
			WARN,
			ERR,
			NONE
		};

		LogType type{ LogType::INFO };
		std::string log{ "" };
	};


	class Logger
	{

	public:
		//Instance
		static Logger& GetInstance();

		~Logger() = default;
		//Makes the logger non-copyable
		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;

		void Init(bool consoleLog = true, bool retainLogs = true);

		template <typename... Args>
		void Log(const std::string& message, Args&&... args);

		template <typename... Args>
		void Warn(const std::string& message, Args&&... args);

		template <typename... Args>
		void Error(std::source_location location, const std::string& message, Args&&... args);

	private:
		std::mutex m_Mutex;
		std::vector<LogEntry> m_LogEntries;
		bool m_bLogAdded{ false };
		bool m_bInitialized{ false };
		bool m_bConsoleLog{ true };
		bool m_bRetainLogs{ true };

		Logger() = default;

		struct LogTime
		{
			std::string day, dayNumber, month, year, time;
			LogTime(const std::string& date);
		};

		std::string CurrentDateTime();

	};


} // namespace ARK_LOGGER

#include "Logger.inl"