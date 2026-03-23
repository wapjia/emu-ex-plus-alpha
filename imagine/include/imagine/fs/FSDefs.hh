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
#include <imagine/util/string/StaticString.hh>
#include <imagine/util/string/CStringView.hh>
#include <imagine/util/string/uri.hh>
#include <unistd.h>
#include <limits.h>
#ifndef IG_USE_MODULE_STD
#include <chrono>
#include <array>
#include <algorithm>
#include <string_view>
#include <utility>
#endif

namespace IG::FS
{

class directory_entry;
class AssetDirectoryIterator;

using file_time_type = std::chrono::system_clock::time_point;

inline constexpr size_t FILE_STRING_SIZE = std::max(512, NAME_MAX + 1);
using FileString = StaticString<FILE_STRING_SIZE - 1>;

inline constexpr size_t PATH_STRING_SIZE = std::max(1024, PATH_MAX);
using PathString = StaticString<PATH_STRING_SIZE - 1>;

using FileStringArray = std::array<char, FILE_STRING_SIZE>;
using PathStringArray = std::array<char, PATH_STRING_SIZE>;

template <class T>
concept ConvertibleToPathString = std::convertible_to<T, PathString> || std::convertible_to<T, std::string_view>;

struct RootPathInfo
{
	size_t length{};
	FileString name;

	constexpr RootPathInfo() = default;
	constexpr RootPathInfo(auto &&name, size_t length):
		length{length}, name{IG_forward(name)} {}
};

struct RootedPath
{
	PathString path;
	RootPathInfo info;

	constexpr bool pathIsRoot() const
	{
		return path.size() == info.length;
	}
};

struct PathLocation
{
	RootedPath root;
	FileString description;

	constexpr PathLocation() = default;
	constexpr PathLocation(auto &&path, auto &&description, auto &&rootName):
		root{IG_forward(path), {IG_forward(rootName), path.size()}}, description{IG_forward(description)} {}
	constexpr PathLocation(auto &&path, auto &&description):
		PathLocation(IG_forward(path), IG_forward(description), IG_forward(description)) {}

	explicit constexpr operator bool() const
	{
		return root.path.size();
	}
};

enum class file_type : int8_t
{
	none = 0,
	not_found = -1,
	regular = 1,
	directory = 2,
	symlink = 3,
	block = 4,
	character = 5,
	fifo = 6,
	socket = 7,
	unknown = 8
};

enum class acc
{
	e = F_OK,
	r = R_OK,
	w = W_OK,
	x = X_OK,
	rw = r | w,
	rx = r | x,
	wx = w | x,
	rwx = r | w | x
};

class file_status
{
public:
	constexpr file_status() = default;
	constexpr file_status(file_type type, std::uintmax_t size, file_time_type lastWriteTime):
		size_{size}, lastWriteTime_{lastWriteTime}, type_{type} {}
	constexpr file_type type() const { return type_; }
	constexpr std::uintmax_t size() const { return size_; }
	constexpr file_time_type lastWriteTime() const { return lastWriteTime_; }

protected:
	std::uintmax_t size_{};
	file_time_type lastWriteTime_{};
	file_type type_ = file_type::none;
};

struct DirOpenFlags
{
	 uint8_t
	// return from constructor without throwing exception if opening fails
	test:1{};

	constexpr bool operator ==(DirOpenFlags const &) const = default;
};

PathString makeAppPathFromLaunchCommand(CStringView launchPath);
FileString basename(CStringView path);
PathString dirname(CStringView path);
FileString displayName(CStringView path);

inline constexpr PathString pathString(ConvertibleToPathString auto &&base, auto &&...components)
{
	PathString path{IG_forward(base)};
	([&]()
	{
		path += '/';
		path += IG_forward(components);
	}(), ...);
	return path;
}

// URI path functions
inline constexpr std::string_view uriPathSegmentTreeName{"/tree/"};
inline constexpr std::string_view uriPathSegmentDocumentName{"/document/"};
PathString dirnameUri(CStringView pathOrUri);
std::pair<std::string_view, size_t> uriPathSegment(std::string_view uri, std::string_view segmentName);

inline constexpr PathString uriString(ConvertibleToPathString auto &&base, auto &&...components)
{
	if(!isUri(base))
		return pathString(IG_forward(base), IG_forward(components)...);
	// assumes base is already encoded and encodes the components
	PathString uri{IG_forward(base)};
	([&]()
	{
		uri += "%2F";
		uri += encodeUri<PathString>(IG_forward(components));
	}(), ...);
	return uri;
}

}
