#include "Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <memory>


static std::unique_ptr<std::vector<LogEntry>> logEntries = nullptr;
static std::unique_ptr<std::mutex> logMutex = nullptr;

void Logger::ensureInitialized()
{
    if (!logEntries) {
        static std::mutex initMutex;
        std::lock_guard<std::mutex> lock(initMutex);
        if (!logEntries) {
            logEntries = std::make_unique<std::vector<LogEntry>>();
            logMutex = std::make_unique<std::mutex>();
        }
    }
}

std::string Logger::getCurrentTime() // for getting time 
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;

#ifdef _MSC_VER
    std::tm timeinfo;
    localtime_s(&timeinfo, &time_t);
    ss << std::put_time(&timeinfo, "[%d-%m-%Y] [%H:%M:%S");
#else
    ss << std::put_time(std::localtime(&time_t), "[%d-%m-%Y] [%H:%M:%S");
#endif

    ss << ":" << std::setfill('0') << std::setw(3) << ms.count() << "]";
    return ss.str();
}

void Logger::addLogEntry(LogLevel level, const std::string& message)
{
    ensureInitialized();

    if (logMutex && logEntries) {
        std::lock_guard<std::mutex> lock(*logMutex);
        logEntries->push_back({ getCurrentTime(), level, message });

        // Prevent unbounded growth
        const size_t MAX_LOG_ENTRIES = 1000;
        if (logEntries->size() > MAX_LOG_ENTRIES) {
            logEntries->erase(logEntries->begin());
        }
    }
}

std::vector<LogEntry> Logger::getLogEntries()
{
    ensureInitialized();

    if (logMutex && logEntries) {
        std::lock_guard<std::mutex> lock(*logMutex);
        return *logEntries; // return copy
    }
    return {};
}

void Logger::clearLogs()
{
    ensureInitialized();

    if (logMutex && logEntries) {
        std::lock_guard<std::mutex> lock(*logMutex);
        logEntries->clear();
    }
}

void Logger::cleanup()
{
    if (logMutex) {
        std::lock_guard<std::mutex> lock(*logMutex);
        if (logEntries) {
            logEntries->clear();
        }
    }
    logEntries.reset();
    logMutex.reset();
}
