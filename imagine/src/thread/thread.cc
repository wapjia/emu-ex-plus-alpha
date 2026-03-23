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

#include <imagine/thread/Thread.hh>
#include <imagine/logger/SystemLogger.hh>
#ifdef __linux__
#include <sched.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#else
#include <pthread.h>
#endif
import std;

namespace IG
{

[[maybe_unused]] static SystemLogger log{"Thread"};

void setThreadCPUAffinityMask([[maybe_unused]] std::span<const ThreadId> ids, [[maybe_unused]] CPUMask mask)
{
	#ifdef __linux__
	cpu_set_t cpuSet{};
	if(mask)
	{
		std::memcpy(&cpuSet, &mask, sizeof(mask));
	}
	else
	{
		std::memset(&cpuSet, 0xFF, sizeof(cpuSet));
	}
	for(auto id : ids)
	{
		// using direct syscall for compatibility with old Android versions
		if(syscall(__NR_sched_setaffinity, id, sizeof(cpuSet), &cpuSet) && Config::DEBUG_BUILD)
			log.error("error:{} setting thread:{:X} CPU affinity", std::strerror(errno), id);
	}
	#endif
}

void setThreadPriority([[maybe_unused]] ThreadId id, [[maybe_unused]] int nice)
{
	#ifdef __linux__
	if(setpriority(PRIO_PROCESS, id, nice) && Config::DEBUG_BUILD)
		log.error("error:{} setting thread:{} nice level:{}", std::strerror(errno), id, nice);
	#endif
}

void setThisThreadPriority([[maybe_unused]] int nice)
{
	#ifdef __linux__
	setThreadPriority(0, nice);
	#endif
}

int thisThreadPriority()
{
	#ifdef __linux__
	return getpriority(PRIO_PROCESS, 0);
	#else
	return 0;
	#endif
}

ThreadId thisThreadId()
{
	#ifdef __linux__
	return gettid();
	#else
	uint64_t id{};
	pthread_threadid_np(nullptr, &id);
	return id;
	#endif
}

}
