#pragma once

/*  This file is part of 2600.emu.

	2600.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	2600.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with 2600.emu.  If not, see <http://www.gnu.org/licenses/> */

#include "bspf.hxx"
#include <utility>

class FSNode
{
public:
	constexpr FSNode() = default;
	template<class T>
	explicit FSNode(T&& path): path{std::forward<T>(path)} {}

  friend ostream& operator<<(ostream& os, const FSNode& node)
  {
    return os << node.getPath();
  }

	bool exists() const;
	string_view getName() const;
	string_view getPath() const;
	bool isDirectory() const;
	bool isFile() const;
	bool isReadable() const;
	bool isWritable() const;
	size_t read(ByteBuffer& buffer, size_t size = 0) const { return 0; }
	size_t read(stringstream& buffer) const { return 0; }
	size_t write(const ByteBuffer& buffer, size_t size) const { return 0; }
	size_t write(const stringstream& buffer) const { return 0; }
	string getNameWithExt(string_view ext) const;
	string getPathWithExt(string_view ext) const;

protected:
	string path;
};
