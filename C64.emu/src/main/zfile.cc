/*  This file is part of C64.emu.

	C64.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	C64.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with C64.emu.  If not, see <http://www.gnu.org/licenses/> */

module;
extern "C"
{
	#include "zfile.h"
}

module system;

using namespace IG;
using namespace EmuEx;

extern "C"
{

FILE* zfile_fopen(const char* path_, const char* mode_)
{
	using namespace IG;
	std::string_view path{path_};
	std::string_view mode{mode_};
	auto appContext = gAppContext();
	if(path == ":::N") // null output
	{
		size_t *buffSizePtr;
		std::ranges::copy(std::span{path_ + 5, sizeof(buffSizePtr)}, asWritableBytes(buffSizePtr).begin());
		//C64System::log.info("decoded size ptr:{}", (void*)buffSizePtr);
		return OutSizeTracker{buffSizePtr}.toFileStream(mode_);
	}
	else if(path == ":::B") // buffer input/output
	{
		size_t *buffSizePtr;
		auto it = std::ranges::copy(std::span{path_ + 5, sizeof(buffSizePtr)}, asWritableBytes(buffSizePtr).begin()).in;
		uint8_t *buffDataPtr;
		std::ranges::copy(std::span{it, sizeof(buffDataPtr)}, asWritableBytes(buffDataPtr).begin());
		//C64System::log.info("decoded size ptr:{} ({}) buff ptr:{}", (void*)buffSizePtr, *buffSizePtr, (void*)buffDataPtr);
		return MapIO{IOBuffer{std::span<uint8_t>{buffDataPtr, *buffSizePtr},
			[buffSizePtr](const uint8_t *, size_t size){ *buffSizePtr = size; }}}.toFileStream(mode_);
	}
	else if(EmuApp::hasArchiveExtension(appContext.fileUriDisplayName(path)))
	{
		if(mode.contains('w') || mode.contains('+'))
		{
			C64System::log.error("opening archive {} with write mode not supported", path);
			return nullptr;
		}
		try
		{
			for(auto &entry : FS::ArchiveIterator{appContext.openFileUri(path)})
			{
				if(entry.type() == FS::file_type::directory)
				{
					continue;
				}
				if(AppMeta::defaultFsFilter(entry.name()))
				{
					C64System::log.info("archive file entry:{}", entry.name());
					return MapIO{std::move(entry)}.toFileStream(mode_);
				}
			}
			C64System::log.error("no recognized file extensions in archive:{}", path);
		}
		catch(...)
		{
			C64System::log.error("error opening archive:{}", path);
		}
		return nullptr;
	}
	else
	{
		return FileUtils::fopenUri(appContext, path, mode_);
	}
}

off_t archdep_file_size(FILE* stream)
{
	off_t pos = ftello(stream);
	if(pos == -1)
		return -1;
	fseeko(stream, 0, SEEK_END);
	off_t end = ftello(stream);
	fseeko(stream, pos, SEEK_SET);
	return end;
}

}
