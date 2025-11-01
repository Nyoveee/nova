#pragma once
#include "export.h"
#include <format>
#include <string>
#include <string_view>
#include <vector>

// Log levels
enum class LogLevel {
	Info,
	Warning,
	Debug,
	Error
};

// single log entry contains this 
struct LogEntry {
	std::string timestamp;
	LogLevel level;
	std::string message;
};

class Logger
{
private:
	FRAMEWORK_DLL_API static void ensureInitialized();
	FRAMEWORK_DLL_API static void addLogEntry(LogLevel level, const std::string& message);

public:
	FRAMEWORK_DLL_API static std::string getCurrentTime();
	FRAMEWORK_DLL_API static std::string getUniqueTimedId();
	FRAMEWORK_DLL_API static std::vector<LogEntry> getLogEntries();
	FRAMEWORK_DLL_API static void clearLogs();
	FRAMEWORK_DLL_API static void cleanup();

	// log function
	template<typename ...Args>
	static void warn(std::string_view rt_fmt_str, Args&&... args);
	template<typename ...Args>
	static void info(std::string_view rt_fmt_str, Args&&... args);
	template<typename ...Args>
	static void error(std::string_view rt_fmt_str, Args&&... args);
	template<typename ...Args>
	static void debug(std::string_view rt_fmt_str, Args&&... args);
	
	//static void log(LogLevel level, const std::string& message);
};

#include "Logger.ipp"
