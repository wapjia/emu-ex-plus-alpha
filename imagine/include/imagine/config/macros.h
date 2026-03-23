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

#ifdef IMAGINE_CONFIG_H
#define IMAGINE_CONFIG_H_INCLUDE <IMAGINE_CONFIG_H>
#include IMAGINE_CONFIG_H_INCLUDE
#undef IMAGINE_CONFIG_H_INCLUDE
#else
#include <imagine-config.h>
#endif

#ifdef NDEBUG
#define IMAGINE_VERSION IMAGINE_VERSION_BASE
#else
#define IMAGINE_VERSION IMAGINE_VERSION_BASE "D"
#endif

#include <imagine/util/macros.h>

#if defined __APPLE__
#include <TargetConditionals.h>
	#if TARGET_OS_IPHONE
	#define CONFIG_OS_IOS
		#if !defined __ARM_ARCH_6K__
		#define CONFIG_INPUT_APPLE_GAME_CONTROLLER 1
		#endif
	#endif
#endif
#ifdef __ANDROID__
#include <android/api-level.h>
#endif

#if defined __ANDROID__
	#ifdef ANDROID_COMPAT_API
	#define ANDROID_MIN_API ANDROID_COMPAT_API
	#else
	#define ANDROID_MIN_API __ANDROID_API__
	#endif

#define ENV_NOTE "Android"
#endif

// Platform architecture & machine

#if defined __x86_64__
#define CONFIG_ARCH_STR "x86_64"
#elif defined __i386__
#define CONFIG_ARCH_STR "x86"
#elif defined __powerpc__
#define CONFIG_ARCH_STR "ppc"
#elif defined __mips__
#define CONFIG_ARCH_STR "mips"
#elif defined __aarch64__
#define CONFIG_ARCH_STR "aarch64"
#elif __arm__
	#if defined __ARM_ARCH_7S__
	#define CONFIG_ARCH_STR "armv7s"
	#elif defined __ARM_ARCH_5TE__
	// default Android "ARM" profile
	#define CONFIG_ARCH_STR "armv5te"
	#elif __ARM_ARCH == 7
	// default Android & iOS ARMv7 profile -> __ARM_ARCH_7A__
	#define CONFIG_ARCH_STR "armv7"
	#elif __ARM_ARCH == 6
	// default iOS ARMv6 profile -> __ARM_ARCH_6K__
	#define CONFIG_ARCH_STR "armv6"
	#else
	#define CONFIG_ARCH_STR "arm"
	#endif
#else
#warning Compiling on unknown architecture
#endif


#if defined CONFIG_PACKAGE_X11 || defined __ANDROID__ || defined _WIN32
#define CONFIG_INPUT_GAMEPAD_DEVICES
#endif

#if defined CONFIG_PACKAGE_X11 || defined __ANDROID__ || defined __APPLE__
#define CONFIG_INPUT_BLUETOOTH
#endif

#if defined __ANDROID__
#define CONFIG_BLUETOOTH_ANDROID
#elif defined __APPLE__ && TARGET_OS_IPHONE
#define CONFIG_BLUETOOTH_BTSTACK
#elif defined __linux__
#define CONFIG_BLUETOOTH_BLUEZ
#endif

#if defined CONFIG_BLUETOOTH_BLUEZ || defined CONFIG_BLUETOOTH_BTSTACK
#define CONFIG_BLUETOOTH_SERVER
#endif

#if defined __ANDROID__ || defined CONFIG_OS_IOS
#define CONFIG_BASE_MULTI_SCREEN
#endif

#if defined CONFIG_OS_IOS && defined __ARM_ARCH_6K__
#define CONFIG_GFX_SOFT_ORIENTATION 1
#elif !defined __ANDROID__ && !defined CONFIG_OS_IOS
#define CONFIG_GFX_SOFT_ORIENTATION 1
#endif

#if defined __ANDROID__
#define IG_CONFIG_SENSORS
#endif

#if defined __linux__
#define CONFIG_BASE_GL_PLATFORM_EGL
#endif

#ifndef CONFIG_GFX_OPENGL_ES
	#if defined CONFIG_OS_IOS || defined __ANDROID__ || defined CONFIG_MACHINE_PANDORA
	#define CONFIG_GFX_OPENGL_ES 2
	#endif
#endif
