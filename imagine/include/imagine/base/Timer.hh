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
#include <imagine/base/baseDefs.hh>
#include <imagine/base/EventLoop.hh>
#include <imagine/util/concepts.hh>

#if defined __linux__
#include <imagine/base/timer/TimerFD.hh>
#elif defined __APPLE__
#include <imagine/base/timer/CFTimer.hh>
#endif

#include <chrono>
#include <string_view>

namespace IG
{

struct TimerDesc
{
	std::string_view debugLabel{};
	EventLoop eventLoop{};
};

struct Timer : public TimerImpl
{
public:
	using Duration = TimePoint::duration;

	constexpr Timer() = default;
	Timer(TimerDesc desc, CallbackDelegate del): TimerImpl{desc, del} {}
	void run(Duration timeUntilRun, Duration repeatInterval, bool isAbsoluteTime = false, CallbackDelegate c = {});
	void cancel();
	void setCallback(CallbackDelegate);
	void setEventLoop(EventLoop);
	void unsetEventLoop();
	void dispatchEarly();
	bool isArmed() const;
	Duration timeUntilRun() const;
	explicit operator bool() const;

	void runIn(ChronoDuration auto timeUntilRun,
		ChronoDuration auto repeatInterval,
		CallbackDelegate f = {})
	{
		run(timeUntilRun, repeatInterval, false, f);
	}

	void runAt(TimePoint time,
		ChronoDuration auto repeatInterval,
		CallbackDelegate f = {})
	{
		run(time.time_since_epoch(), repeatInterval, true, f);
	}

	// non-repeating timer
	void runIn(ChronoDuration auto timeUntilRun, CallbackDelegate f = {})
	{
		run(timeUntilRun, Duration{}, false, f);
	}

	void runAt(TimePoint time, CallbackDelegate f = {})
	{
		run(time.time_since_epoch(), Duration{}, true, f);
	}
};

}
