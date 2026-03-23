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

#include <imagine/base/baseDefs.hh>
#include <imagine/time/Time.hh>
#include <imagine/util/used.hh>
#include <CoreFoundation/CoreFoundation.h>
#ifndef IG_USE_MODULE_STD
#include <memory>
#include <string_view>
#endif

namespace IG
{

struct CFTimerInfo
{
	CallbackDelegate callback{};
	CFRunLoopRef loop{};
	CFRunLoopRef setLoop{};
};

class CFTimer
{
public:
	using TimePoint = SteadyClockTimePoint;

	constexpr CFTimer() = default;
	CFTimer(TimerDesc, CallbackDelegate);
	CFTimer(CFTimer&&) noexcept;
	CFTimer &operator=(CFTimer&&) noexcept;
	~CFTimer();
	std::string_view debugLabel() const { return debugLabel_; }

protected:
	ConditionalMember<Config::DEBUG_BUILD, std::string_view> debugLabel_{};
	CFRunLoopTimerRef timer{};
	std::unique_ptr<CFTimerInfo> info;

	void callbackInCFAbsoluteTime(CFAbsoluteTime absTime, CFTimeInterval repeatInterval, CFRunLoopRef loop);
	void deinit();
};

using TimerImpl = CFTimer;

}
