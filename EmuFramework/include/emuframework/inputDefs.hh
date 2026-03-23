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
import imagine;
import std;
#else
#include <imagine/input/inputDefs.hh>
#include <imagine/util/container/array.hh>
#include <imagine/util/concepts.hh>
#include <imagine/util/2DOrigin.h>
#include <array>
#include <cstdint>
#include <span>
#endif

namespace EmuEx
{

using namespace IG;

struct KeyFlags
{
	uint8_t
	appCode:1{},
	turbo:1{},
	toggle:1{},
	unused:1{},
	deviceId:4{};

	constexpr bool operator==(const KeyFlags &) const = default;
};

using KeyCode = uint8_t;
using KeyCodeArray = ZArray<KeyCode, 3>;

inline constexpr KeyCode comboKeyCode = 255;

struct KeyInfo
{
	KeyCodeArray codes;
	KeyFlags flags;

	constexpr KeyInfo() = default;

	constexpr KeyInfo(NotPointer auto code, KeyFlags flags = {}):
		codes{KeyCode(code)}, flags{flags} {}

	template <NotPointer T>
	constexpr KeyInfo(std::array<T, 2> codes, KeyFlags flags = {}):
		codes{KeyCode(codes[0]), KeyCode(codes[1])}, flags{flags} {}

	template <NotPointer T>
	constexpr KeyInfo(std::array<T, 3> codes, KeyFlags flags = {}):
		codes{KeyCode(codes[0]), KeyCode(codes[1]), KeyCode(codes[2])}, flags{flags} {}

	static constexpr auto appKey(auto code)
	{
		return KeyInfo{code, KeyFlags{.appCode = 1}};
	}

	static constexpr auto comboKey(KeyCode idx)
	{
		return KeyInfo{std::array{comboKeyCode, idx}, KeyFlags{.appCode = 1}};
	}

	constexpr bool isAppKey() const { return flags.appCode; }
	constexpr bool isComboKey() const { return isAppKey() && codes[0] == comboKeyCode; }
	constexpr bool operator==(const KeyInfo &) const = default;
	constexpr explicit operator bool() { return codes[0]; }
	constexpr auto &operator[](size_t pos) { return codes[pos]; }
	constexpr auto &operator[](size_t pos) const { return codes[pos]; }
};

using MappedKeys = ZArray<Input::Key, 3>;

struct KeyMapping
{
	KeyInfo key{};
	MappedKeys mapKey{};

	constexpr KeyMapping() = default;
	constexpr KeyMapping(KeyInfo key, Input::Key mapKey):
		key{key}, mapKey{mapKey} {}
	constexpr KeyMapping(KeyInfo key, MappedKeys mapKey):
		key{key}, mapKey{mapKey} {}
};

struct InputAction
{
	KeyCode code{};
	KeyFlags flags{};
	Input::Action state{};
	uint32_t metaState{};

	constexpr bool isPushed() const { return state == Input::Action::PUSHED; }
	constexpr operator KeyInfo() const { return KeyInfo{code, flags}; }
};

enum class InputComponent : uint8_t
{
	ui, dPad, button, trigger
};

struct InputComponentFlags
{
	uint8_t
	altConfig:1{},
	rowSize:2{},
	staggeredLayout:1{};
};

struct InputComponentDesc
{
	const char *name{};
	std::span<const KeyInfo> keyCodes{};
	InputComponent type{};
	_2DOrigin layoutOrigin{};
	InputComponentFlags flags{};
};

struct SystemInputDeviceDesc
{
	const char *name;
	std::span<const InputComponentDesc> components;
};

inline constexpr int VControllerVKeyCols = 20;
inline constexpr int VControllerKeyRows = 4;
inline constexpr int VControllerKeyCols = VControllerVKeyCols / 2;
using VControllerKbMap = std::array<KeyInfo, VControllerKeyRows * VControllerKeyCols>;

enum class VControllerKbMode: uint8_t
{
	LAYOUT_1,
	LAYOUT_2
};

}
