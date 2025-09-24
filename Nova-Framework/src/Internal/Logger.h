#pragma once
#include "export.h"
#include <format>
#include <string>

namespace Logger
{
	FRAMEWORK_DLL_API std::string getCurrentTime();
	template<typename ...Args>
	inline void warn(std::string_view rt_fmt_str, Args&&... args);
	template<typename ...Args>
	inline void info(std::string_view rt_fmt_str, Args&&... args);
	template<typename ...Args>
	inline void error(std::string_view rt_fmt_str, Args&&... args);
}

#include "Logger.ipp"