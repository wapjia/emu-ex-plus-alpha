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

#include <imagine/base/ApplicationContext.hh>
#include <imagine/base/Application.hh>
#include <imagine/base/Screen.hh>
#include <imagine/logger/logger.h>
#include <imagine/time/Time.hh>
#include <imagine/util/utility.h>
#include <imagine/util/algorithm.h>

namespace IG
{

constexpr SystemLogger log{"Screen"};

Screen::Screen(ApplicationContext ctx, InitParams params):
	ScreenImpl{ctx, params},
	windowsPtr{&ctx.application().windows()},
	appCtx{ctx}
{
	ctx.application().emplaceFrameTimer(frameTimer, *this);
}

void Screen::addOnFrame(OnFrameDelegate del, int priority, InsertMode mode)
{
	postFrame();
	onFrameDelegate.insert(del, priority, mode);
}

bool Screen::removeOnFrame(OnFrameDelegate del, DelegateFuncEqualsMode mode)
{
	bool removed = onFrameDelegate.removeFirst(del, mode);
	return removed;
}

bool Screen::containsOnFrame(OnFrameDelegate del) const
{
	return onFrameDelegate.contains(del);
}

void Screen::runOnFrameDelegates(SteadyClockTimePoint time)
{
	FrameParams params
	{
		.time = time, .lastTime = std::exchange(lastFrameTime_, time),
		.duration = frameTimerRate().duration(), .mode = FrameClockMode::screen
	};
	onFrameDelegate.runAll([&](OnFrameDelegate del)
	{
		return del(params);
	});
}

size_t Screen::onFrameDelegates() const
{
	return onFrameDelegate.size();
}

bool Screen::isPosted() const
{
	return framePosted;
}

bool Screen::frameUpdate(SteadyClockTimePoint time)
{
	assert(hasTime(time));
	assert(isActive);
	framePosted = false;
	if(!onFrameDelegate.size())
	{
		lastFrameTime_ = {};
		return false;
	}
	postFrame();
	runOnFrameDelegates(time);
	for(auto &w : *windowsPtr)
	{
		if(w->screen() == this)
		{
			w->dispatchOnDraw();
		}
	}
	return true;
}

void Screen::setActive(bool active)
{
	if(active && !isActive)
	{
		log.info("screen:{} activated", (void*)this);
		isActive = true;
		if(onFrameDelegate.size())
			postFrame();
	}
	else if(!active && isActive)
	{
		log.info("screen:{} deactivated", (void*)this);
		isActive = false;
		unpostFrame();
	}
}

void Screen::postFrame()
{
	if(!isActive) [[unlikely]]
	{
		log.info("skipped posting inactive screen:{}", (void*)this);
		return;
	}
	if(framePosted)
		return;
	//log.info("posting frame");
	framePosted = true;
	frameTimer.scheduleVSync();
}

void Screen::unpostFrame()
{
	if(!framePosted)
		return;
	framePosted = false;
	lastFrameTime_ = {};
	frameTimer.cancel();
}

bool Screen::shouldUpdateFrameTimer(const FrameTimer& frameTimer, bool newVariableFrameTimeValue)
{
	return (newVariableFrameTimeValue && !std::holds_alternative<SimpleFrameTimer>(frameTimer)) ||
		(!newVariableFrameTimeValue && std::holds_alternative<SimpleFrameTimer>(frameTimer));
}

[[gnu::weak]] SteadyClockDuration Screen::targetFrameDuration() const { return frameRate().duration(); }

FrameRate Screen::frameTimerRate() const { return frameTimer.frameRate() ?: frameRate(); }

void Screen::setVariableFrameRate(bool useVariableTime)
{
	if(!shouldUpdateFrameTimer(frameTimer, useVariableTime))
		return;
	application().emplaceFrameTimer(frameTimer, *this, useVariableTime);
}

void Screen::setFrameEventsOnThisThread()
{
	frameTimer.setEventsOnThisThread(appContext());
}

void Screen::removeFrameEvents()
{
	unpostFrame();
	frameTimer.removeEvents(appContext());
}

[[gnu::weak]] void Screen::setFrameInterval([[maybe_unused]] int interval) {}
[[gnu::weak]] bool Screen::supportsFrameInterval() { return false; }

}
