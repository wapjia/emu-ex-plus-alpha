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

#ifdef IG_USE_MODULES
#define MAGIC_ENUM_USE_STD_MODULE
import std;
#else
#include <utility>
#include <type_traits>
#endif
#include <imagine/util/magic_enum/magic_enum.hpp>

namespace IG
{

#ifndef IG_USE_MODULE_IMAGINE
using std::to_underlying;
#endif

template <class T>
concept Enum = std::is_enum_v<T>;

template <Enum T>
constexpr auto lastEnum = magic_enum::enum_values<T>().back();

template <Enum T>
constexpr auto enumCount = magic_enum::enum_count<T>();

constexpr auto enumName(Enum auto e) { return magic_enum::enum_name(e); }

template<Enum T>
constexpr bool enumIsValid(T v)
{
	return magic_enum::enum_contains<T>(v);
}

}
