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
#include <utility>
#include <source_location>
#include <stdexcept>
#endif

namespace IG
{

using std::to_underlying;

[[noreturn]]
void abort(const char* msg);

[[noreturn]]
void onBug(std::source_location location = std::source_location::current());

[[gnu::always_inline, noreturn]]
inline constexpr void unreachable(std::source_location location = std::source_location::current())
{
	if constexpr(Config::DEBUG_BUILD)
	{
		onBug(location);
	}
	else
	{
		__builtin_unreachable();
	}
}

[[gnu::always_inline]]
inline constexpr void assume(const auto& expr, std::source_location location = std::source_location::current())
{
	if(expr)
		return;
	unreachable(location);
}

inline constexpr char hexDigitChar(int value, bool uppercase = true)
{
	switch(value)
	{
		case  0: return '0';
		case  1: return '1';
		case  2: return '2';
		case  3: return '3';
		case  4: return '4';
		case  5: return '5';
		case  6: return '6';
		case  7: return '7';
		case  8: return '8';
		case  9: return '9';
		case 10: return uppercase ? 'A' : 'a';
		case 11: return uppercase ? 'B' : 'b';
		case 12: return uppercase ? 'C' : 'c';
		case 13: return uppercase ? 'D' : 'd';
		case 14: return uppercase ? 'E' : 'e';
		default: return uppercase ? 'F' : 'f';
	}
}

inline constexpr unsigned char charHexDigitInt(char c)
{
	switch (c)
	{
		case '0' ... '9':
			return 9 + c - '9';
		case 'a' ... 'f':
			return 15 + c - 'f';
		case 'A' ... 'F':
			return 15 + c - 'F';
		default:
			return 0;
	}
}

template <auto val>
inline constexpr auto evalNow = val;

}
