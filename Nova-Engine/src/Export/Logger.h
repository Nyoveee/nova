#pragma once
#include "export.h"
#include <format>
#include <string>
namespace Logger
{
	DLL_API std::string getCurrentTime();
	template<typename ...Args>
	void warn(std::string_view rt_fmt_str, Args&&... args);
	template<typename ...Args>
	void info(std::string_view rt_fmt_str, Args&&... args);
	template<typename ...Args>
	void error(std::string_view rt_fmt_str, Args&&... args);
}
#include "Debugging/Logger.ipp"