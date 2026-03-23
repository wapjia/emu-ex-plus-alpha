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

#include <imagine/fs/FS.hh>
#include <imagine/fs/FSUtils.hh>
#include <imagine/base/ApplicationContext.hh>
#include <imagine/util/format.hh>
#include <imagine/logger/SystemLogger.hh>
#include <stdlib.h>
#if defined __linux__ && !defined __ANDROID__
#include <libgen.h>
#undef basename
#include <string.h> // use GNU version of basename() that doesn't modify argument
#define DIRNAME_MODIFIES_ARG true
#else
// Bionic or BSD
#include <libgen.h>
#define DIRNAME_MODIFIES_ARG false
#endif

namespace IG::FS
{

static SystemLogger log{"FS"};
constexpr bool dirnameCanModifyArgument = DIRNAME_MODIFIES_ARG;

static auto basenameImpl(const char *path)
{
	return [](auto path)
	{
		if constexpr(requires {::basename(path);})
		{
			// Bionic or GNU C versions take const char*
			return ::basename(path);
		}
		else
		{
			// BSD version takes char *, but always returns its own allocated storage
			return ::basename(PathString{path}.data());
		}
	}(path);
}

static auto dirnameImpl(const char *path)
{
	return [](auto path)
	{
		if constexpr(requires {::dirname(path);})
		{
			// Bionic version takes const char*
			return ::dirname(path);
		}
		else
		{
			if constexpr(dirnameCanModifyArgument)
			{
				// standard version can modify input, and returns a pointer within it
				PathString tempPath{path};
				FileString output{::dirname(tempPath.data())};
				return output;
			}
			else
			{
				// BSD version takes char *, but always returns its own allocated storage
				return ::dirname(PathString{path}.data());
			}
		}
	}(path);
}

// make sure basename macro doesn't leak
#undef basename

PathString makeAppPathFromLaunchCommand(CStringView launchCmd)
{
	log.info("app path from launch command:{}", launchCmd);
	PathStringArray realPath;
	if(!realpath(FS::dirname(launchCmd).data(), realPath.data()))
	{
		log.error("error in realpath()");
		return {};
	}
	return realPath.data();
}

FileString basename(CStringView path)
{
	return basenameImpl(path);
}

PathString dirname(CStringView path)
{
	return dirnameImpl(path);
}

FileString displayName(CStringView path)
{
	if(path.empty() || !FS::exists(path))
		return {};
	else
		return basename(path);
}

PathString dirnameUri(CStringView pathOrUri)
{
	if(pathOrUri.empty())
		return {};
	if(!isUri(pathOrUri))
		return dirname(pathOrUri);
	if(auto [treePath, treePos] = FS::uriPathSegment(pathOrUri, FS::uriPathSegmentTreeName);
		Config::envIsAndroid && treePos != std::string_view::npos)
	{
		auto [docPath, docPos] = FS::uriPathSegment(pathOrUri, FS::uriPathSegmentDocumentName);
		if(docPos == std::string_view::npos)
		{
			log.error("invalid document path in tree URI:{}", pathOrUri);
			return {};
		}
		if(auto lastSlashPos = docPath.rfind("%2F");
			lastSlashPos != std::string_view::npos) // return everything before the last /
		{
			return {pathOrUri, docPos + lastSlashPos};
		}
		if(auto colonPos = docPath.find("%3A");
			colonPos != std::string_view::npos) // at root, return everything before and including the :
		{
			colonPos += 3;
			return {pathOrUri, docPos + colonPos};
		}
	}
	log.error("can't get directory name on unsupported URI:{}", pathOrUri);
	return {};
}

std::pair<std::string_view, size_t> uriPathSegment(std::string_view uri, std::string_view name)
{
	assume(name.starts_with('/') && name.ends_with('/'));
	auto pathPos = uri.find(name);
	if(pathPos == std::string_view::npos)
		return {{}, std::string_view::npos};
	pathPos += name.size();
	auto pathStart = uri.substr(pathPos);
	// return the substring of the segment and the absolute offset into the original string view
	return {pathStart.substr(0, pathStart.find('/')), pathPos};
}

size_t directoryItems(CStringView path)
{
	size_t items = 0;
	forEachInDirectory(path, [&](auto&){ items++; return true; });
	return items;
}

bool forEachInDirectory(CStringView path, DirectoryEntryDelegate del, DirOpenFlags flags)
{
	bool entriesRead{};
	for(FS::DirectoryStream dirStream{path, flags}; dirStream.hasEntry(); dirStream.readNextDir())
	{
		entriesRead = true;
		if(!del(dirStream.entry()))
			break;
	}
	return entriesRead;
}

std::string formatLastWriteTimeLocal(ApplicationContext ctx, CStringView path)
{
	return ctx.formatDateAndTime(status(path).lastWriteTime());
}

}
