#pragma once
#include <iostream>

namespace Logger
{
	template<typename ...Args>
	void warn(std::string_view rt_fmt_str, Args && ...args)
	{
		std::cout << getCurrentTime()
			<< " [Warning] " << std::vformat(rt_fmt_str, std::make_format_args(args...)) << std::endl;
	}
	template<typename ...Args>
	void Logger::info(std::string_view rt_fmt_str, Args && ...args)
	{
		std::cout << getCurrentTime()
			<< " [Info] " << std::vformat(rt_fmt_str, std::make_format_args(args...)) << std::endl;
	}
	template<typename ...Args>
	void error(std::string_view rt_fmt_str, Args && ...args)
	{
		std::cout <<getCurrentTime()
			<< " [Error] " << std::vformat(rt_fmt_str, std::make_format_args(args...)) << std::endl;
	}
}