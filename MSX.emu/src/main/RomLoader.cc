/*  This file is part of MSX.emu.

	MSX.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	MSX.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with MSX.emu.  If not, see <http://www.gnu.org/licenses/> */

module;
#include "ziphelper.h"

extern "C"
{
	#include <blueMSX/Memory/RomLoader.h>
}

module system;

namespace EmuEx
{

ArchiveIO &MsxSystem::firmwareArchive(CStringView path) const
{
	if(!firmwareArch)
	{
		log.info("{} not cached, opening archive", path);
		firmwareArch = {appContext().openFileUri(path)};
	}
	else
	{
		firmwareArch.rewind();
	}
	return firmwareArch;
}

}

using namespace IG;
using namespace EmuEx;

static UInt8 *fileToMallocBuffer(Readable auto &file, int *size)
{
	int fileSize = file.size();
	auto buff = (UInt8*)std::malloc(fileSize);
	file.read(buff, fileSize);
	*size = fileSize;
	return buff;
}

static IO fileFromFirmwarePath(CStringView path)
{
	auto &sys = static_cast<MsxSystem&>(gSystem());
	auto appCtx = sys.appContext();
	auto firmwarePath = sys.firmwarePath();
	if(firmwarePath.size())
	{
		try
		{
			if(FS::hasArchiveExtension(firmwarePath))
			{
				auto &arch = sys.firmwareArchive(firmwarePath);
				if(FS::seekFileInArchive(arch, [&](auto &entry){ return entry.name().ends_with(path.data()); }))
				{
					return MapIO{arch};
				}
			}
			else
			{
				return appCtx.openFileUri(FS::uriString(firmwarePath, path), {.accessHint = IOAccessHint::All});
			}
		}
		catch(...)
		{
			MsxSystem::log.error("error opening path:{}", path);
		}
	}
	// fall back to asset path
	auto assetFile = appCtx.openAsset(path, {.test = true, .accessHint = IOAccessHint::All});
	if(assetFile)
	{
		return assetFile;
	}
	MsxSystem::log.error("{} not found in firmware path", path);
	return {};
}

extern "C" UInt8* romLoad(const char* filename, const char* filenameInArchive, int* size)
{
	if(!filename || !std::strlen(filename))
		return nullptr;
	if(filenameInArchive && std::strlen(filenameInArchive))
	{
		MsxSystem::log.info("loading zipped ROM:{}:{}", filename, filenameInArchive);
		auto buff = (UInt8*)zipLoadFile(filename, filenameInArchive, size);
		if(buff)
			return buff;
		MsxSystem::log.error("can't load ROM from zip");
		return nullptr;
	}
	else
	{
		MsxSystem::log.info("loading ROM:{}", filename);
		auto &sys = static_cast<MsxSystem&>(gSystem());
		auto appCtx = sys.appContext();
		if(filename[0] == '/' || isUri(filename)) // try to load absolute path directly
		{
			auto file = appCtx.openFileUri(filename, {.test = true, .accessHint = IOAccessHint::All});
			if(file)
			{
				return fileToMallocBuffer(file, size);
			}
			MsxSystem::log.error("can't load ROM from absolute path");
			return nullptr;
		}
		// relative path, try firmware directory
		{
			auto file = fileFromFirmwarePath(filename);
			if(file)
			{
				return fileToMallocBuffer(file, size);
			}
		}
		MsxSystem::log.error("can't load ROM from relative path");
		return nullptr;
	}
}

extern "C" FILE *openMachineIni(const char *filename, const char *mode)
{
	MsxSystem::log.info("loading machine ini:{}", filename);
	auto file = fileFromFirmwarePath(filename);
	if(file)
	{
		return MapIO{std::move(file)}.toFileStream(mode);
	}
	return {};
}
