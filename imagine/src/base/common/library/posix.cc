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

#include <imagine/base/sharedLibrary.hh>
#include <imagine/logger/SystemLogger.hh>
#include <dlfcn.h>

namespace IG
{

static SystemLogger log{"App"};

SharedLibraryRef openSharedLibrary(const char *name, OpenSharedLibraryFlags flags)
{
	int mode = flags.resolveAllSymbols ? RTLD_NOW : RTLD_LAZY;
	auto lib = dlopen(name, mode);
	if(Config::DEBUG_BUILD && !lib)
	{
		log.error("dlopen({}) error:{}", name, dlerror());
	}
	return lib;
}

void closeSharedLibrary(SharedLibraryRef lib)
{
	dlclose(lib);
}

void *loadSymbol(SharedLibraryRef lib, const char *name)
{
	if(!lib)
		lib = RTLD_DEFAULT;
	return dlsym(lib, name);
}

const char *lastOpenSharedLibraryError()
{
	return dlerror();
}

}
