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
#include <imagine/fs/PosixFS.hh>
#include <imagine/util/string/CStringView.hh>
#ifndef IG_USE_MODULE_STD
#include <concepts>
#include <cstddef>
#include <memory>
#include <iterator>
#include <string_view>
#include <utility>
#endif

// Tries to mirror API of C++ filesystem TS library in most cases

namespace IG::FS
{

class directory_iterator
{
public:
	using iterator_category = std::input_iterator_tag;
	using value_type = directory_entry;
	using difference_type = ptrdiff_t;
	using pointer = value_type*;
	using reference = value_type&;

	constexpr directory_iterator() = default;
	directory_iterator(CStringView path);
	directory_iterator(const directory_iterator&) = default;
	directory_iterator(directory_iterator&&) = default;
	directory_entry& operator*();
	directory_entry* operator->();
	void operator++();
	bool operator==(directory_iterator const &rhs) const;

protected:
	std::shared_ptr<DirectoryStream> impl;
};

inline const directory_iterator &begin(const directory_iterator& iter)
{
	return iter;
}

inline directory_iterator end(const directory_iterator&)
{
	return {};
}

PathString current_path();
void current_path(CStringView path);
bool exists(CStringView path);
std::uintmax_t file_size(CStringView path);
file_status status(CStringView path);
file_status symlink_status(CStringView path);
void chown(CStringView path, uid_t owner, gid_t group);
bool access(CStringView path, acc type);
bool remove(CStringView path);
bool create_directory(CStringView path);
bool rename(CStringView oldPath, CStringView newPath);

inline PathString createDirectorySegments(ConvertibleToPathString auto &&base, auto &&...components)
{
	PathString path{IG_forward(base)};
	([&]()
	{
		path += '/';
		path += IG_forward(components);
		create_directory(path);
	}(), ...);
	return path;
}

}

namespace IG
{
	// convenience aliases for path strings
	using FS::PathString;
	using FS::FileString;
	using FS::pathString;
	using FS::uriString;
}
