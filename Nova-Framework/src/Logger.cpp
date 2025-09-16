#include "Logger.h"  
#include <sstream>
#include <chrono>

std::string Logger::getCurrentTime()
{
    constexpr char logFormatString[] = "[%d-%m-%Y] [%H:%I:%S]";

    std::ostringstream ss;
    auto start = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(start);
#pragma warning(disable:4996)
    ss << std::put_time(std::localtime(&time), logFormatString);
#pragma warning(default:4996)
    return ss.str();
}
