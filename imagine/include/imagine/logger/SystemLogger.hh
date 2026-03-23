#pragma once

/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#include <imagine/config/defs.hh>
#ifndef IG_USE_MODULE_STD
#include <format>
#endif

namespace IG::Log
{

enum class Level
{
	E, // Error
	W, // Warning
	I, // Info
	D, // Debug Info
};

void print(Level, std::string_view tag, std::string_view format, std::format_args);
void printMsg(Level, const char* str, size_t strSize);
void setLogDirectoryPrefix(const char *dirStr);
void setEnabled(bool enable);
bool isEnabled();

constexpr auto toColorCode(Level lv)
{
	switch(lv)
	{
		case Level::D: return "\033[1;36m";
		default: [[fallthrough]];
		case Level::I: return "\033[0m";
		case Level::W: return "\033[1;33m";
		case Level::E: return "\033[1;31m";
	}
}

inline void beginMsg(auto& str, Level lv, std::string_view tag, std::string_view format, std::format_args args)
{
	if(Config::envIsLinux)
	{
		str += toColorCode(lv);
	}
	if(tag.size())
	{
		str += tag;
		str += ": ";
	}
	std::vformat_to(std::back_inserter(str), format, args);
}

}

namespace IG
{

class SystemLogger
{
public:
	std::string_view tag;

	template <class... T>
	void print(Log::Level lv, std::format_string<T...> format, T&&... args) const
	{
		Log::print(lv, tag, format.get(), std::make_format_args(args...));
	}

	template <class... T>
	void info(std::format_string<T...> format, T&&... args) const
	{
		Log::print(Log::Level::I, tag, format.get(), std::make_format_args(args...));
	}

	template <class... T>
	void debug(std::format_string<T...> format, T&&... args) const
	{
		Log::print(Log::Level::D, tag, format.get(), std::make_format_args(args...));
	}

	template <class... T>
	void warn(std::format_string<T...> format, T&&... args) const
	{
		Log::print(Log::Level::W, tag, format.get(), std::make_format_args(args...));
	}

	template <class... T>
	void error(std::format_string<T...> format, T&&... args) const
	{
		Log::print(Log::Level::E, tag, format.get(), std::make_format_args(args...));
	}

	template <class... T>
	void beginMsg(auto& str, Log::Level lv, std::format_string<T...> format, T&&... args) const
	{
		Log::beginMsg(str, lv, tag, format.get(), std::make_format_args(args...));
	}

	template <class... T>
	void beginInfo(auto& str, std::format_string<T...> format, T&&... args) const
	{
		Log::beginMsg(str, Log::Level::I, tag, format.get(), std::make_format_args(args...));
	}

	template <class... T>
	void beginDebug(auto& str, std::format_string<T...> format, T&&... args) const
	{
		Log::beginMsg(str, Log::Level::D, tag, format.get(), std::make_format_args(args...));
	}

	template <class... T>
	void beginWarn(auto& str, std::format_string<T...> format, T&&... args) const
	{
		Log::beginMsg(str, Log::Level::W, tag, format.get(), std::make_format_args(args...));
	}

	template <class... T>
	void beginError(auto& str, std::format_string<T...> format, T&&... args) const
	{
		Log::beginMsg(str, Log::Level::E, tag, format.get(), std::make_format_args(args...));
	}

	void printMsg(Log::Level lv, auto& str) const
	{
		Log::printMsg(lv, str.c_str(), str.size());
	}

	void printInfo(auto& str) const { printMsg(Log::Level::I, str); }
	void printDebug(auto& str) const { printMsg(Log::Level::D, str); }
	void printWarn(auto& str) const { printMsg(Log::Level::W, str); }
	void printError(auto& str) const { printMsg(Log::Level::E, str); }
};

}
