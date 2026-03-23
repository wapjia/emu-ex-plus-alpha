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

#ifdef CONFIG_PACKAGE_X11
#include <imagine/base/x11/inputDefs.hh>
#elif defined __ANDROID__
#include <imagine/base/android/inputDefs.hh>
#elif defined CONFIG_OS_IOS
#include <imagine/base/iphone/inputDefs.hh>
#elif defined CONFIG_BASE_MACOSX
#include <imagine/base/osx/inputDefs.hh>
#elif defined CONFIG_BASE_WIN32
#include <imagine/base/win32/inputDefs.hh>
#endif

#ifndef IG_USE_MODULE_STD
#include <string>
#include <string_view>
#include <memory>
#endif

namespace IG
{

struct InputAxisFlags
{
	using BitSetClassInt = uint32_t;

	BitSetClassInt
	x:1{},  y:1{},  z:1{},
	rx:1{}, ry:1{}, rz:1{},
	hatX:1{}, hatY:1{},
	lTrigger:1{}, rTrigger:1{},
	rudder:1{}, wheel:1{},
	gas:1{}, brake:1{};

	constexpr bool operator==(InputAxisFlags const&) const = default;
};

struct InputDeviceTypeFlags
{
	using BitSetClassInt = uint8_t;

	BitSetClassInt
	miscKeys:1{},
	keyboard:1{},
	gamepad:1{},
	joystick:1{},
	virtualInput:1{},
	mouse:1{},
	touchscreen:1{},
	powerButton:1{};

	constexpr bool operator==(InputDeviceTypeFlags const&) const = default;
};

}

namespace IG::Input
{

class Event;
class Device;
struct AxisKeyEmu;
class TextField;
class MogaManager;

using DeviceTypeFlags = InputDeviceTypeFlags;
using AxisFlags = InputAxisFlags;

enum class Source : uint8_t
{
	UNKNOWN,
	KEYBOARD,
	GAMEPAD,
	MOUSE,
	TOUCHSCREEN,
	NAVIGATION,
	JOYSTICK,
};

enum class Map : uint8_t
{
	UNKNOWN = 0,
	SYSTEM = 1,
	POINTER = 2,
	REL_POINTER = 3,

	WIIMOTE = 10,
	WII_CC = 11,

	ICONTROLPAD = 20,
	ZEEMOTE = 21,
	PS3PAD = 23,

	APPLE_GAME_CONTROLLER = 31
};

enum class DeviceSubtype : uint8_t
{
	NONE = 0,
	XPERIA_PLAY = 1,
	PS3_CONTROLLER = 2,
	OUYA_CONTROLLER = 4,
	PANDORA_HANDHELD = 5,
	GENERIC_GAMEPAD = 8,
	APPLE_EXTENDED_GAMEPAD = 9,
};

enum class Action : uint8_t
{
	UNUSED,
	RELEASED,
	PUSHED,
	MOVED,
	MOVED_RELATIVE,
	EXIT_VIEW,
	ENTER_VIEW,
	SCROLL_UP,
	SCROLL_DOWN,
	CANCELED,
};

using PointerId = PointerIdImpl;

inline constexpr PointerId NULL_POINTER_ID
{
	[]()
	{
		if constexpr(std::is_pointer_v<PointerId>)
			return nullptr;
		else
			return -1;
	}()
};

struct OuyaKey
{
	static constexpr Key
	O = Keycode::GAME_A,
	U = Keycode::GAME_X,
	Y = Keycode::GAME_Y,
	A = Keycode::GAME_B,
	L1 = Keycode::GAME_L1,
	L2 = Keycode::GAME_L2,
	L3 = Keycode::GAME_LEFT_THUMB,
	R1 = Keycode::GAME_R1,
	R2 = Keycode::GAME_R2,
	R3 = Keycode::GAME_RIGHT_THUMB,
	UP = Keycode::UP, RIGHT = Keycode::RIGHT, DOWN = Keycode::DOWN, LEFT = Keycode::LEFT,
	SYSTEM = Keycode::MENU;
};

struct PS3Key
{
	static constexpr Key
	CROSS = Keycode::GAME_X,
	CIRCLE = Keycode::GAME_Y,
	SQUARE = Keycode::GAME_A,
	TRIANGLE = Keycode::GAME_B,
	L1 = Keycode::GAME_L1,
	L2 = Keycode::GAME_L2,
	L3 = Keycode::GAME_LEFT_THUMB,
	R1 = Keycode::GAME_R1,
	R2 = Keycode::GAME_R2,
	R3 = Keycode::GAME_RIGHT_THUMB,
	SELECT = Keycode::GAME_SELECT,
	START = Keycode::GAME_START,
	UP = Keycode::UP, RIGHT = Keycode::RIGHT, DOWN = Keycode::DOWN, LEFT = Keycode::LEFT,
	PS = Keycode::GAME_1;
};

enum class AxisSetId : uint8_t
{
	stick1,
	stick2,
	hat,
	triggers,
	pedals
};

inline constexpr DeviceTypeFlags virtualDeviceFlags{.miscKeys = true, .keyboard = true, .virtualInput = true};

class BaseDevice
{
public:
	friend class Device;
	using Subtype = DeviceSubtype;

	BaseDevice() = default;
	BaseDevice(int id, Map map, DeviceTypeFlags, std::string name);
	bool operator==(BaseDevice const&) const = default;

protected:
	std::shared_ptr<void> appDataPtr;
	std::string name_;
	int id_{};
	DeviceTypeFlags typeFlags_{};
	uint8_t enumId_{};
	Map map_{Map::UNKNOWN};
	Subtype subtype_{};

	void updateGamepadSubtype(std::string_view name, uint32_t vendorProductId);
};

}
