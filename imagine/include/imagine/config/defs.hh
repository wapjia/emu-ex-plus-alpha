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

#include <imagine/config/macros.h>

namespace Config
{

// TODO: have to use ANDROID_ for now since ANDROID is needed as a macro in some system headers not yet using __ANDROID__
enum class Env { ANDROID_, IOS, MACOSX, LINUX, WIN32_ };
inline constexpr Env ENV =
	#if defined __ANDROID__
	Env::ANDROID_;
	#elif defined __APPLE__ && TARGET_OS_IPHONE
	Env::IOS;
	#elif defined __APPLE__ && TARGET_OS_MAC
	Env::MACOSX;
	#elif defined __linux__
	Env::LINUX;
	#elif defined _WIN32
	Env::WIN32_;
	#else
	#error "Unknown ENV type"
	#endif

inline constexpr bool envIsAndroid = ENV == Env::ANDROID_;
inline constexpr bool envIsIOS = ENV == Env::IOS;
inline constexpr bool envIsMacOSX = ENV == Env::MACOSX;
inline constexpr bool envIsLinux = ENV == Env::LINUX;
inline constexpr bool is64Bit = sizeof(void*) == 8;

#if defined __ANDROID__
inline constexpr int ENV_ANDROID_MIN_SDK = ANDROID_MIN_API;
#else
inline constexpr int ENV_ANDROID_MIN_SDK = 0;
#endif

#ifndef NDEBUG
inline constexpr bool DEBUG_BUILD = true;
#else
inline constexpr bool DEBUG_BUILD = false;
#endif

// Platform architecture & machine

enum class Machine
{
	GENERIC_X86,
	GENERIC_X86_64,
	GENERIC_ARM,
	GENERIC_ARMV5,
	GENERIC_ARMV6,
	GENERIC_ARMV7,
	GENERIC_ARMV7S,
	GENERIC_AARCH64,
	GENERIC_PPC,
	GENERIC_MIPS,
	PANDORA,
};

#if defined __x86_64__
inline constexpr Machine MACHINE = Machine::GENERIC_X86_64;
#elif defined __i386__
inline constexpr Machine MACHINE = Machine::GENERIC_X86;
#elif defined __powerpc__
inline constexpr Machine MACHINE = Machine::GENERIC_PPC;
#elif defined __mips__
inline constexpr Machine MACHINE = Machine::GENERIC_MIPS;
#elif defined __aarch64__
inline constexpr Machine MACHINE = Machine::GENERIC_AARCH64;
#elif __arm__
	#if defined __ARM_ARCH_7S__
	inline constexpr Machine MACHINE = Machine::GENERIC_ARMV7S;
	#elif defined __ARM_ARCH_5TE__
	// default Android "ARM" profile
	inline constexpr Machine MACHINE = Machine::GENERIC_ARMV5;
	#elif __ARM_ARCH == 7
	// default Android & iOS ARMv7 profile -> __ARM_ARCH_7A__
		#if defined CONFIG_MACHINE_PANDORA
		inline constexpr Machine MACHINE = Machine::PANDORA;
		#else
		inline constexpr Machine MACHINE = Machine::GENERIC_ARMV7;
		#endif
	#elif __ARM_ARCH == 6
	// default iOS ARMv6 profile -> __ARM_ARCH_6K__
	inline constexpr Machine MACHINE = Machine::GENERIC_ARMV6;
	#else
	inline constexpr Machine MACHINE = Machine::GENERIC_ARM;
	#endif
#else
#error "Compiling on unknown architecture"
#endif

#ifdef __ARM_ARCH
inline constexpr int ARM_ARCH = __ARM_ARCH;
#else
inline constexpr int ARM_ARCH = 0;
#endif

inline constexpr bool MACHINE_IS_GENERIC_X86 = MACHINE == Machine::GENERIC_X86;
inline constexpr bool MACHINE_IS_GENERIC_ARMV6 = MACHINE == Machine::GENERIC_ARMV6;
inline constexpr bool MACHINE_IS_GENERIC_ARMV7 = MACHINE == Machine::GENERIC_ARMV7;
inline constexpr bool MACHINE_IS_GENERIC_AARCH64 = MACHINE == Machine::GENERIC_AARCH64;
inline constexpr bool MACHINE_IS_PANDORA = MACHINE == Machine::PANDORA;

	namespace Input
	{
	#if (defined __APPLE__ && TARGET_OS_IPHONE) || defined __ANDROID__
	inline constexpr bool SYSTEM_COLLECTS_TEXT = true;
	#else
	inline constexpr bool SYSTEM_COLLECTS_TEXT = false;
	#endif

	// dynamic input device list from system
	#if defined CONFIG_PACKAGE_X11 || defined __ANDROID__ || defined __APPLE__
	inline constexpr bool DEVICE_HOTSWAP = true;
	#else
	inline constexpr bool DEVICE_HOTSWAP = false;
	#endif

	inline constexpr bool KEYBOARD_DEVICES = true;

	// mouse & touch
	inline constexpr bool POINTING_DEVICES = true;

	#if defined CONFIG_PACKAGE_X11 || defined __ANDROID__ || defined _WIN32
	inline constexpr bool MOUSE_DEVICES = true;
	#else
	inline constexpr bool MOUSE_DEVICES = false;
	#endif

	#if defined CONFIG_PACKAGE_X11 || defined __ANDROID__ || defined _WIN32
	inline constexpr bool GAMEPAD_DEVICES = true;
	#else
	inline constexpr bool GAMEPAD_DEVICES = false;
	#endif

	#if (defined __APPLE__ && TARGET_OS_IPHONE) || defined __ANDROID__
	inline constexpr bool TOUCH_DEVICES = true;
	#else
	inline constexpr bool TOUCH_DEVICES = false;
	#endif

	inline constexpr int MAX_POINTERS =
	#if defined CONFIG_PACKAGE_X11
	4; // arbitrary max
	#elif defined CONFIG_OS_IOS || defined __ANDROID__
	// arbitrary max
	7;
	#else
	1;
	#endif

	// relative motion/trackballs
	#ifdef __ANDROID__
	inline constexpr bool RELATIVE_MOTION_DEVICES = true;
	#else
	inline constexpr bool RELATIVE_MOTION_DEVICES = false;
	#endif

	#if defined CONFIG_PACKAGE_X11 || defined __ANDROID__ || defined __APPLE__
	inline constexpr bool BLUETOOTH = true;
	#else
	inline constexpr bool BLUETOOTH = false;
	#endif
	}

}
