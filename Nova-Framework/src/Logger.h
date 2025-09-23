//#pragma once
//#include "export.h"
//#include <format>
//#include <string>
//#include <string_view>
//#include <vector>
//#include <mutex>
//#include <memory>
//
//enum class LogLevel {
//    Info,
//    Warning,
//    Error
//};
//
//struct LogEntry {
//    std::string timestamp;
//    LogLevel level;
//    std::string message;
//};
//
//class DLL_API Logger
//{
//private:
//    // Use smart pointers to avoid static initialization/destruction issues
//    static std::unique_ptr<std::vector<LogEntry>> logEntries;
//    static std::unique_ptr<std::mutex> logMutex;
//    static void ensureInitialized();
//    static void addLogEntry(LogLevel level, const std::string& message);
//
//public:
//    static std::string getCurrentTime();
//    static std::vector<LogEntry> getLogEntries(); // Return by value to avoid reference issues
//    static void clearLogs();
//    static void cleanup(); // Explicit cleanup function
//
//    template<typename ...Args>
//    static void warn(std::string_view rt_fmt_str, Args&&... args);
//    template<typename ...Args>
//    static void info(std::string_view rt_fmt_str, Args&&... args);
//    template<typename ...Args>
//    static void error(std::string_view rt_fmt_str, Args&&... args);
//};
//
//#include "Logger.ipp"
//


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
    Error
};

// single log entry contains this 
struct LogEntry {
    std::string timestamp;
    LogLevel level;
    std::string message;
};

class DLL_API Logger
{
private:
    
    static void ensureInitialized();
    static void addLogEntry(LogLevel level, const std::string& message);

public:
    static std::string getCurrentTime();
    static std::vector<LogEntry> getLogEntries(); 
    static void clearLogs();
    static void cleanup();

    // log function
    template<typename ...Args>
    static void warn(std::string_view rt_fmt_str, Args&&... args);
    template<typename ...Args>
    static void info(std::string_view rt_fmt_str, Args&&... args);
    template<typename ...Args>
    static void error(std::string_view rt_fmt_str, Args&&... args);
};

#include "Logger.ipp"
