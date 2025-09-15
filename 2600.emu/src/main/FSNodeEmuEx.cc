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

#include <FSNode.hxx>

bool FSNode::exists() const
{
	return true;
}

string_view FSNode::getName() const
{
	return path;
}

string_view FSNode::getPath() const
{
	return path;
}

bool FSNode::isReadable() const
{
	return true;
}

bool FSNode::isWritable() const
{
	return true;
}

bool FSNode::isDirectory() const
{
	return false;
}

bool FSNode::isFile() const
{
	return true;
}

string FSNode::getNameWithExt(string_view ext) const
{
	size_t pos = getName().find_last_of("/\\");
	string s = pos == string::npos ? string{getName()} : string{getName().substr(pos+1)};
	pos = s.find_last_of('.');
	return (pos != string::npos) ? s.replace(pos, string::npos, ext) : s + ext;
}

string FSNode::getPathWithExt(string_view ext) const
{
	string s = path;
	const size_t pos = s.find_last_of('.');
	return (pos != string::npos) ? s.replace(pos, string::npos, ext) : s + ext;
}
