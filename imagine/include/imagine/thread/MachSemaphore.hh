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

#include <mach/semaphore.h>
#include <mach/task.h>
#include <mach/mach.h>
#include <imagine/util/utility.hh>
#ifndef IG_USE_MODULE_STD
#include <chrono>
#endif

// TODO: remove when <semaphore> is fully supported by Apple Clang

namespace IG
{

template<unsigned LeastMaxValue>
class MachSemaphore
{
public:
	MachSemaphore(unsigned startValue)
	{
		[[maybe_unused]] auto ret = semaphore_create(mach_task_self(), &sem, SYNC_POLICY_FIFO, startValue);
		assume(ret == KERN_SUCCESS);
	}

	MachSemaphore(MachSemaphore &&o) noexcept
	{
		*this = std::move(o);
	}

	MachSemaphore &operator=(MachSemaphore &&o) noexcept
	{
		deinit();
		sem = std::exchange(o.sem, {});
		return *this;
	}

	~MachSemaphore()
	{
		deinit();
	}

	void acquire()
	{
		semaphore_wait(sem);
	}

	template<class Rep, class Period>
	bool try_acquire_for(const std::chrono::duration<Rep, Period>&)
	{
		semaphore_wait(sem); // TODO
		return true;
	}

	void release()
	{
		semaphore_signal(sem);
	}

protected:
	semaphore_t sem{};

	void deinit()
	{
		if(!sem)
			return;
		[[maybe_unused]] auto ret = semaphore_destroy(mach_task_self(), sem);
		assume(ret == KERN_SUCCESS);
		sem = {};
	}
};

template<unsigned LeastMaxValue>
using counting_semaphore = MachSemaphore<LeastMaxValue>;
using binary_semaphore = MachSemaphore<1>;

}
