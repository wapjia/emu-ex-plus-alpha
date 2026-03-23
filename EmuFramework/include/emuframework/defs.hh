#pragma once

/*  This file is part of EmuFramework.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with EmuFramework.  If not, see <http://www.gnu.org/licenses/> */

#ifdef IG_USE_MODULES
#include <format>
import imagine;
import std;
#else
#include <imagine/config/defs.hh>
#include <imagine/bluetooth/defs.hh>
#include <imagine/util/Point2D.hh>
#include <cstdint>
#include <concepts>
#include <string_view>
#endif

namespace IG
{
class BasicNavView;
}

namespace EmuEx
{

class EmuApp;
class EmuSystem;
class EmuVideo;
class EmuAudio;
using NameFilterFunc = bool(*)(std::string_view name);
using AppNavView = IG::BasicNavView;

inline constexpr bool MOGA_INPUT = Config::envIsAndroid;
inline constexpr bool CAN_HIDE_TITLE_BAR = !Config::envIsIOS;
inline constexpr bool enableFullFrameTimingStats = Config::DEBUG_BUILD;
inline constexpr bool hasICadeInput = Config::Input::KEYBOARD_DEVICES;

enum class AltSpeedMode
{
	fast, slow
};

enum class OutputFrameRateMode: std::uint8_t
{
	Auto, Detect, Screen
};

enum class CPUAffinityMode: std::uint8_t
{
	Auto, Any, Manual
};

struct AudioStats
{
	int underruns{};
	int overruns{};
	int callbacks{};
	double avgCallbackFrames{};
	int frames{};
};

struct AspectRatioInfo
{
	std::string_view name{};
	IG::Point2D<int8_t> aspect{};

	constexpr float asFloat() const { return aspect.ratio<float>(); }
};

struct BundledGameInfo
{
	const char *displayName;
	const char *assetName;
};

enum class ViewID
{
	MAIN_MENU,
	SYSTEM_ACTIONS,
	VIDEO_OPTIONS,
	AUDIO_OPTIONS,
	SYSTEM_OPTIONS,
	FILE_PATH_OPTIONS,
	GUI_OPTIONS,
};

inline constexpr float menuVideoBrightnessScale = .25f;

}
