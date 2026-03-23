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

#include <imagine/base/Screen.hh>
#include <imagine/base/Application.hh>
#include <imagine/logger/SystemLogger.hh>
#include <xcb/randr.h>
#include "macros.h"
import xutils;

namespace IG
{

static SystemLogger log{"X11Screen"};

XScreen::XScreen(ApplicationContext, InitParams params)
{
	auto &screen = params.screen;
	xScreen = &screen;
	auto &conn = params.conn;
	xMM = screen.width_in_millimeters;
	yMM = screen.height_in_millimeters;
	if(Config::MACHINE_IS_PANDORA)
	{
		// TODO: read actual frame rate value
		frameRate_ = 60.;
	}
	else
	{
		frameRate_ = 60.;
		reliableFrameTime = false;
		xcb_randr_output_t primaryOutput{};
		auto resReply = XCB_REPLY(xcb_randr_get_screen_resources, &conn, screen.root);
		if(resReply)
		{
			auto outPrimaryReply = XCB_REPLY(xcb_randr_get_output_primary, &conn, screen.root);
			if(outPrimaryReply)
			{
				primaryOutput = outPrimaryReply->output;
			}
			else
			{
				for(auto output : std::span<xcb_randr_output_t>{xcb_randr_get_screen_resources_outputs(resReply.get()),
					size_t(xcb_randr_get_screen_resources_outputs_length(resReply.get()))})
				{
					auto outputInfoReply = XCB_REPLY(xcb_randr_get_output_info, &conn, output, resReply->timestamp);
					if(outputInfoReply && outputInfoReply->connection == XCB_RANDR_CONNECTION_CONNECTED)
					{
						primaryOutput = output;
						break;
					}
				}
			}
			auto outputInfoReply = XCB_REPLY(xcb_randr_get_output_info, &conn, primaryOutput, resReply->timestamp);
			if(outputInfoReply)
			{
				auto crtcInfoReply = XCB_REPLY(xcb_randr_get_crtc_info, &conn, outputInfoReply->crtc, outputInfoReply->timestamp);
				if(crtcInfoReply)
				{
					for(auto &modeInfo : std::span<xcb_randr_mode_info_t>{xcb_randr_get_screen_resources_modes(resReply.get()),
						(size_t)xcb_randr_get_screen_resources_modes_length(resReply.get())})
					{
						if(modeInfo.id == crtcInfoReply->mode && modeInfo.htotal && modeInfo.vtotal)
						{
							frameRate_ = {double(modeInfo.dot_clock) / (modeInfo.htotal * modeInfo.vtotal),
								fromSeconds<SteadyClockDuration>(modeInfo.htotal * modeInfo.vtotal / double(modeInfo.dot_clock))};
							reliableFrameTime = true;
							break;
						}
					}
				}
			}
		}
		assume(frameRate_.hz());
	}
	log.info("screen:{} {}x{} ({}x{}mm) {}Hz", (void*)&screen,
		screen.width_in_pixels, screen.height_in_pixels, (int)xMM, (int)yMM, frameRate_.hz());
}

xcb_screen_t* XScreen::nativeObject() const
{
	return xScreen;
}

std::pair<float, float> XScreen::mmSize() const
{
	return {xMM, yMM};
}

bool XScreen::operator ==(XScreen const &rhs) const
{
	return xScreen == rhs.xScreen;
}

XScreen::operator bool() const
{
	return xScreen;
}

int Screen::width() const
{
	return xScreen->width_in_pixels;
}

int Screen::height() const
{
	return xScreen->height_in_pixels;
}

FrameRate Screen::frameRate() const { return frameRate_; }

bool Screen::frameRateIsReliable() const
{
	return reliableFrameTime;
}

void Screen::setFrameRate(FrameRate rate)
{
	if constexpr(Config::MACHINE_IS_PANDORA)
	{
		if(!rate)
			rate = 60;
		else
			rate = std::round(rate.hz());
		if(rate.hz() != 50 && rate.hz() != 60)
		{
			log.warn("tried to set unsupported frame rate:{}", rate.hz());
			return;
		}
		auto cmd = std::format("sudo /usr/pandora/scripts/op_lcdrate.sh {}", (unsigned int)rate.hz());
		int err = std::system(cmd.data());
		if(err)
		{
			log.error("error:{} setting frame rate", err);
			return;
		}
		frameRate_ = rate;
		frameTimer.setFrameRate(rate);
	}
	else
	{
		frameTimer.setFrameRate(rate ?: frameRate());
	}
}

bool Screen::supportsTimestamps() const
{
	return application().supportedFrameTimerType() != SupportedFrameTimer::SIMPLE;
}

std::span<const FrameRate> Screen::supportedFrameRates() const
{
	if constexpr(Config::MACHINE_IS_PANDORA)
	{
		static constexpr std::array<FrameRate, 2> rates{50, 60};
		return rates;
	}
	else
	{
		return {&frameRate_, 1};
	}
}

}
