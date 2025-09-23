#pragma once
#include <iostream>
#include "Logger.h"

template<typename ...Args>
void Logger::warn(std::string_view rt_fmt_str, Args&&... args)
{
    std::size_t argSize{ sizeof...(args) };
    std::string message;

    if (argSize == 0) {
        message = std::string(rt_fmt_str);
    }
    else {
        message = std::vformat(rt_fmt_str, std::make_format_args(args...));
    }

    // Print to console 
    std::cout << getCurrentTime() << " [Warning] " << message << std::endl;

    // Add to log entries for display
    addLogEntry(LogLevel::Warning, message);
}

template<typename ...Args>
void Logger::info(std::string_view rt_fmt_str, Args&&... args)
{
    std::size_t argSize{ sizeof...(args) };
    std::string message;

    if (argSize == 0) {
        message = std::string(rt_fmt_str);
    }
    else {
        message = std::vformat(rt_fmt_str, std::make_format_args(args...));
    }

    // Print to console 
    std::cout << getCurrentTime() << " [Info] " << message << std::endl;

    // Add to log entries for display
    addLogEntry(LogLevel::Info, message);
}

template<typename ...Args>
void Logger::error(std::string_view rt_fmt_str, Args&&... args)
{
    std::size_t argSize{ sizeof...(args) };
    std::string message;

    if (argSize == 0) {
        message = std::string(rt_fmt_str);
    }
    else {
        message = std::vformat(rt_fmt_str, std::make_format_args(args...));
    }

    // Print to console 
    std::cout << getCurrentTime() << " [Error] " << message << std::endl;

    // Add to log entries for display
    addLogEntry(LogLevel::Error, message);
}