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

static_assert(__has_feature(objc_arc), "This file requires ARC");
#include <imagine/base/Screen.hh>
#include <imagine/base/ApplicationContext.hh>
#include <imagine/base/Application.hh>
#include <imagine/time/Time.hh>
#include <imagine/logger/SystemLogger.hh>
#include "ios.hh"

namespace IG
{
static SystemLogger log{"Screen"};
}

using namespace IG;

@interface UIScreen ()
- (double)_refreshRate;
@end

@interface DisplayLinkHelper : NSObject
{
@private
	Screen *screen_;
}
@end

@implementation DisplayLinkHelper

- (id)initWithScreen:(Screen *)screen
{
	self = [super init];
	if(self)
	{
		screen_ = screen;
	}
	return self;
}

- (void)onFrame:(CADisplayLink *)displayLink
{
	auto &screen = *screen_;
	auto timestamp = fromSeconds<SteadyClockDuration>(displayLink.timestamp);
	/*IG::log.info("screen:{}, frame time stamp:{}, duration:{}",
		(__bridge void*)screen.uiScreen(), timestamp, displayLink.duration);*/
	if(!screen.frameUpdate(SteadyClockTimePoint{timestamp}))
	{
		displayLink.paused = YES;
	}
}

@end

namespace IG
{

IOSScreen::IOSScreen(ApplicationContext, InitParams initParams):
	uiScreen_{(void*)CFBridgingRetain((__bridge UIScreen*)initParams.uiScreen)}
{
	log.info("init screen:{}", uiScreen_);
	auto currMode = uiScreen().currentMode;
	if(currMode.size.width == 1600 && currMode.size.height == 900)
	{
		log.info("looking for 720p mode to improve non-native video adapter performance");
		for(UIScreenMode *mode in uiScreen().availableModes)
		{
			if(mode.size.width == 1280 && mode.size.height == 720)
			{
				log.info("setting 720p mode");
				uiScreen().currentMode = mode;
				break;
			}
		}
	}
	if(Config::DEBUG_BUILD)
	{
		if(hasAtLeastIOS8())
		{
			log.info("has {} point scaling ({} native)", [uiScreen() scale], [uiScreen() nativeScale]);
		}
		for(UIScreenMode *mode in uiScreen().availableModes)
		{
			log.info("has mode:{}x{}", mode.size.width, mode.size.height);
		}
		log.info("current mode:{}x{}", uiScreen().currentMode.size.width, uiScreen().currentMode.size.height);
		log.info("preferred mode:{}x{}", uiScreen().preferredMode.size.width, uiScreen().preferredMode.size.height);
	}

	// note: the _refreshRate value is actually time per frame in seconds
	auto frameDuration = [uiScreen() _refreshRate];
	if(!frameDuration || 1. / frameDuration < 20. || 1. / frameDuration > 200.)
	{
		log.warn("ignoring unusual refresh rate:{}", 1. / frameDuration);
		frameDuration = 1. / 60.;
	}
	frameRate_ = fromSeconds<SteadyClockDuration>(frameDuration);
}

IOSScreen::~IOSScreen()
{
	log.info("deinit screen:{}", uiScreen_);
	CFRelease(uiScreen_);
}

void Screen::setFrameInterval(int interval)
{
	log.info("setting display interval:{}", interval);
	assert(interval >= 1);
	if(auto timer = std::get_if<DisplayLinkFrameTimer>(&frameTimer); timer)
	{
		[timer->displayLink() setFrameInterval:interval];
	}
}

bool Screen::supportsFrameInterval()
{
	return true;
}

bool Screen::supportsTimestamps() const
{
	return true;
}

int Screen::width() const
{
	return uiScreen().bounds.size.width;
}

int Screen::height() const
{
	return uiScreen().bounds.size.height;
}

FrameRate Screen::frameRate() const { return frameRate_; }

bool Screen::frameRateIsReliable() const
{
	return true;
}

std::span<const FrameRate> Screen::supportedFrameRates() const
{
	// TODO
	return {&frameRate_, 1};
}

DisplayLinkFrameTimer::DisplayLinkFrameTimer(Screen& screen)
{
	displayLink_ = (void*)CFBridgingRetain([screen.uiScreen() displayLinkWithTarget:[[DisplayLinkHelper alloc] initWithScreen:&screen]
	                                       selector:@selector(onFrame:)]);
	displayLink().paused = YES;
	setEventsOnThisThread(screen.appContext());
}

DisplayLinkFrameTimer::~DisplayLinkFrameTimer()
{
	if(!displayLink_)
		return;
	[displayLink() invalidate];
	CFRelease(displayLink_);
	CFRelease(displayLinkRunLoop_);
}

void DisplayLinkFrameTimer::scheduleVSync() { displayLink().paused = NO; }
void DisplayLinkFrameTimer::cancel() { displayLink().paused = YES; }

void DisplayLinkFrameTimer::setEventsOnThisThread(ApplicationContext)
{
	displayLinkRunLoop_ = (void*)CFBridgingRetain([NSRunLoop currentRunLoop]);
	[displayLink() addToRunLoop:displayLinkRunLoop() forMode:NSDefaultRunLoopMode];
}

void DisplayLinkFrameTimer::removeEvents(ApplicationContext)
{
	cancel();
	if(!displayLinkRunLoop_)
		return;
	[displayLink() removeFromRunLoop:displayLinkRunLoop() forMode:NSDefaultRunLoopMode];
	CFRelease(std::exchange(displayLinkRunLoop_, nullptr));
}

void IOSApplication::emplaceFrameTimer(FrameTimer &t, Screen &screen, bool useVariableTime)
{
	if(useVariableTime)
	{
		t.emplace<SimpleFrameTimer>(screen);
	}
	else
	{
		t.emplace<DisplayLinkFrameTimer>(screen);
	}
}

}
