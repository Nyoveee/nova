#pragma once
#include <iostream>

namespace Logger
{
	template<typename ...Args>
	inline void warn(std::string_view rt_fmt_str, Args && ...args)
	{
		std::size_t argSize{ sizeof...(args) };

		std::string text;

		if (argSize == 0) {
			std::cout << getCurrentTime() << " [Warning] " << rt_fmt_str << std::endl;
			text = rt_fmt_str;
		}
		else {
			std::cout << getCurrentTime() << " [Warning] " << std::vformat(rt_fmt_str, std::make_format_args(args...)) << std::endl;
			text = std::vformat(rt_fmt_str, std::make_format_args(args...));
		}


	}
	template<typename ...Args>
	inline void info(std::string_view rt_fmt_str, Args && ...args)
	{
		std::size_t argSize{ sizeof...(args) };
		if (argSize == 0)
			std::cout << getCurrentTime() << " [Info] " << rt_fmt_str << std::endl;
		else
			std::cout << getCurrentTime() << " [Info] " << std::vformat(rt_fmt_str, std::make_format_args(args...)) << std::endl;
		
	}
	template<typename ...Args>
	inline void error(std::string_view rt_fmt_str, Args && ...args)
	{
		std::size_t argSize{ sizeof...(args) };
		if (argSize == 0)
			std::cout << getCurrentTime() << " [Error] " << rt_fmt_str << std::endl;
		else
			std::cout << getCurrentTime() << " [Error] " << std::vformat(rt_fmt_str, std::make_format_args(args...)) << std::endl;
	}
}