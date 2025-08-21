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

#include <imagine/base/SimpleFrameTimer.hh>
#include <imagine/base/Screen.hh>
#include <imagine/time/Time.hh>
#include <imagine/logger/logger.h>

namespace IG
{

constexpr SystemLogger log{"SimpleFrameTimer"};

SimpleFrameTimer::SimpleFrameTimer(Screen &screen, EventLoop loop):
	timer
	{
		{.debugLabel = "SimpleFrameTimer", .eventLoop = loop},
		[this, &screen]()
		{
			if(!requested || !screen.frameUpdate(SteadyClock::now()))
			{
				cancel();
				return false;
			}
			return true;
		}
	},
	rate{screen.frameRate()}
{
	log.info("created frame timer");
}

void SimpleFrameTimer::scheduleVSync()
{
	if(requested)
	{
		return;
	}
	requested = true;
	if(timer.isArmed())
	{
		return;
	}
	assert(rate.hz());
	timer.runIn(Nanoseconds{1}, rate.duration());
}

void SimpleFrameTimer::cancel()
{
	requested = false;
}

void SimpleFrameTimer::setFrameRate(FrameRate rate_)
{
	rate = rate_;
	log.info("set frame rate:{:g} (timer interval:{})", rate.hz(), rate.duration());
	if(timer.isArmed())
	{
		timer.runIn(timer.timeUntilRun(), rate.duration());
	}
}

void SimpleFrameTimer::setEventsOnThisThread(ApplicationContext)
{
	timer.setEventLoop({});
}

void SimpleFrameTimer::removeEvents(ApplicationContext)
{
	cancel();
	timer.unsetEventLoop();
}

}
