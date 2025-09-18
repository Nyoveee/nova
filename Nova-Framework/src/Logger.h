#pragma once
#include "export.h"
#include <format>
#include <string>
#include <string_view>
#include <vector>

class DLL_API Logger
{
public:
    static std::string getCurrentTime();
  

    template<typename ...Args>
    static void warn(std::string_view rt_fmt_str, Args&&... args);

    template<typename ...Args>
    static void info(std::string_view rt_fmt_str, Args&&... args);

    template<typename ...Args>
    static void error(std::string_view rt_fmt_str, Args&&... args);

};


#include "Logger.ipp"