#pragma once
#include <iostream>
#include "Logger.h"

template<typename ...Args>
void Logger::warn(std::string_view rt_fmt_str, Args&&... args)
{
    std::size_t argSize{ sizeof...(args) };

    std::string text;

    if (argSize == 0) {
        std::cout << getCurrentTime() << " [Warning] " << rt_fmt_str << std::endl;
        text = std::string(rt_fmt_str);
    }
    else {
        std::string formatted = std::vformat(rt_fmt_str, std::make_format_args(args...));
        std::cout << getCurrentTime() << " [Warning] " << formatted << std::endl;
        text = formatted;
    }
}

template<typename ...Args>
void Logger::info(std::string_view rt_fmt_str, Args&&... args)
{
    std::size_t argSize{ sizeof...(args) };
    if (argSize == 0) {
        std::cout << getCurrentTime() << " [Info] " << rt_fmt_str << std::endl;
    }
    else {
        std::cout << getCurrentTime() << " [Info] " << std::vformat(rt_fmt_str, std::make_format_args(args...)) << std::endl;
    }
}

template<typename ...Args>
void Logger::error(std::string_view rt_fmt_str, Args&&... args)
{
    std::size_t argSize{ sizeof...(args) };
    if (argSize == 0) {
        std::cout << getCurrentTime() << " [Error] " << rt_fmt_str << std::endl;
    }
    else {
        std::cout << getCurrentTime() << " [Error] " << std::vformat(rt_fmt_str, std::make_format_args(args...)) << std::endl;
    }
}