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
#include <archive.h>
#include <archive_entry.h>
#include "ziphelper.h"

module system;

using namespace IG;
using namespace EmuEx;

static archive* writeArch{};
static FS::ArchiveIterator cachedZipIt{};
static FS::PathString cachedZipName{};
static uint8_t *buffData{};
static size_t buffSize{};

void setZipMemBuffer(std::span<uint8_t> buff)
{
	buffData = buff.data();
	buffSize = buff.size();
}

size_t zipMemBufferSize() { return buffSize; }

static void unsetCachedReadZip()
{
	cachedZipIt = {};
	cachedZipName = {};
	MsxSystem::log.info("unset cached read zip archive");
}

extern "C" void zipCacheReadOnlyZip(const char* zipName_)
{
	if(!zipName_ || !std::strlen(zipName_))
	{
		unsetCachedReadZip();
		return;
	}
	else
	{
		std::string_view zipName{zipName_};
		cachedZipName = zipName;
		if(zipName == ":::B")
		{
			MsxSystem::log.info("using memory buffer as cached read zip archive");
			std::span buff{buffData, buffSize};
			cachedZipIt = IO{buff};
		}
		else
		{
			MsxSystem::log.info("setting cached read zip archive:{}", zipName);
			cachedZipIt = {EmuEx::gAppContext().openFileUri(zipName)};
		}
	}
}

static void* loadFromArchiveIt(FS::ArchiveIterator& it, const char* zipName, const char* fileName, int* size)
{
	for(auto &entry : it)
	{
		if(entry.type() == FS::file_type::directory)
		{
			continue;
		}
		//logMsg("archive file entry:%s", entry.name());
		if(entry.name() == fileName)
		{
			int fileSize = entry.size();
			void *buff = std::malloc(fileSize);
			entry.read(buff, fileSize);
			*size = fileSize;
			return buff;
		}
	}
	MsxSystem::log.error("file {} not in {}archive:{}", fileName,
		cachedZipIt.hasEntry() && cachedZipName == zipName ? "cached " : "", zipName);
	return nullptr;
}

extern "C" void* zipLoadFile(const char* zipName, const char* fileName, int* size)
{
	try
	{
		if(cachedZipIt.hasEntry() && cachedZipName == zipName)
		{
			cachedZipIt.rewind();
			return loadFromArchiveIt(cachedZipIt, zipName, fileName, size);
		}
		else
		{
			auto it = FS::ArchiveIterator{EmuEx::gAppContext().openFileUri(zipName)};
			return loadFromArchiveIt(it, zipName, fileName, size);
		}
	}
	catch(...)
	{
		MsxSystem::log.error("error opening archive:{}", zipName);
		return nullptr;
	}
}

bool zipStartWrite(const char *fileName)
{
	assume(!writeArch);
	writeArch = archive_write_new();
	archive_write_set_format_zip(writeArch);
	if(std::string_view{fileName} == ":::B")
	{
		MsxSystem::log.info("using memory buffer for zip write");
		if(archive_write_open_memory(writeArch, buffData, buffSize, &buffSize) != ARCHIVE_OK)
		{
			archive_write_free(writeArch);
			writeArch = {};
			return false;
		}
	}
	else
	{
		int fd = EmuEx::gAppContext().openFileUriFd(fileName, OpenFlags::testNewFile()).release();
		if(archive_write_open_fd(writeArch, fd) != ARCHIVE_OK)
		{
			archive_write_free(writeArch);
			writeArch = {};
			if(fd != -1)
				::close(fd);
			return false;
		}
	}
	return true;
}

extern "C" int zipSaveFile(const char* zipName, const char* fileName, int append, const void* buffer, int size)
{
	assume(writeArch);
	auto entry = archive_entry_new();
	auto freeEntry = scopeGuard([&](){ archive_entry_free(entry); });
	archive_entry_set_pathname(entry, fileName);
	archive_entry_set_size(entry, size);
	archive_entry_set_filetype(entry, AE_IFREG);
	archive_entry_set_perm(entry, 0644);
	if(archive_write_header(writeArch, entry) < ARCHIVE_OK)
	{
		MsxSystem::log.error("error writing archive header: {}", archive_error_string(writeArch));
		return 0;
	}
	if(archive_write_data(writeArch, buffer, size) < ARCHIVE_OK)
	{
		MsxSystem::log.error("error writing archive data: {}", archive_error_string(writeArch));
		return 0;
	}
	return 1;
}

void zipEndWrite()
{
	assume(writeArch);
	archive_write_close(writeArch);
	archive_write_free(writeArch);
	writeArch = {};
}

extern "C" FILE* fopenHelper(const char* filename, const char* mode)
{
	return FileUtils::fopenUri(EmuEx::gAppContext(), filename, mode);
}
