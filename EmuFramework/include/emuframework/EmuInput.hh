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

#include <emuframework/defs.hh>
#include <emuframework/inputDefs.hh>
#ifdef IG_USE_MODULES
import imagine;
import std;
#else
#include <imagine/input/inputDefs.hh>
#include <imagine/io/FileIO.hh>
#include <imagine/io/MapIO.hh>
#include <imagine/util/used.hh>
#include <string>
#include <string_view>
#include <memory>
#include <span>
#include <algorithm>
#include <vector>
#endif

namespace EmuEx
{

using namespace IG;
class InputDeviceConfig;
struct InputAction;

inline constexpr int8_t playerIndexMulti = -1;
inline constexpr int8_t playerIndexUnset = -2;

struct KeyCategory
{
	std::string_view name;
	std::span<const KeyInfo> keys;
	int multiplayerIndex{}; // if > 0, category appears when one input device is assigned multiple players

	constexpr KeyCategory(std::string_view name, std::span<const KeyInfo> keys, int multiplayerIndex = 0):
		name{name}, keys(keys), multiplayerIndex{multiplayerIndex} {}
	constexpr KeyCategory(std::string_view name, const auto &keys, int multiplayerIndex = 0):
		name{name}, keys{keys.data(), keys.size()}, multiplayerIndex{multiplayerIndex} {}
};

struct KeyConfigDesc
{
	std::string_view name;
	std::span<const KeyMapping> keyMap;
	Input::Map map{};
	Input::DeviceSubtype devSubtype{};

	constexpr KeyConfigDesc() = default;
	constexpr KeyConfigDesc(Input::Map map, Input::DeviceSubtype devSubtype, std::string_view name, std::span<const KeyMapping> keyMap):
		name{name}, keyMap{keyMap}, map{map}, devSubtype{devSubtype} {}
	constexpr KeyConfigDesc(Input::Map map, std::string_view name, std::span<const KeyMapping> keyMap):
		KeyConfigDesc{map, {}, name, keyMap} {}
	constexpr operator bool() { return keyMap.size(); }

	constexpr auto find(KeyInfo key)
	{
		return std::ranges::find_if(keyMap, [&](auto &val){ return val.key == key; });
	}

	constexpr MappedKeys get(KeyInfo key)
	{
		auto it = find(key);
		if(it == keyMap.end())
			return {};
		return it->mapKey;
	}

	constexpr auto findMapped(MappedKeys v)
	{
		return std::ranges::find_if(keyMap, [&](auto &val){ return val.mapKey == v; });
	}

	constexpr KeyInfo getMapped(MappedKeys v)
	{
		auto it = findMapped(v);
		if(it == keyMap.end())
			return {};
		return it->key;
	}
};

class KeyConfig
{
public:
	std::string name;

	constexpr KeyConfig() = default;
	constexpr KeyConfig(Input::Map map, Input::DeviceSubtype devSubtype, std::string_view name, auto &&keyMap):
		name{name}, keyMap{std::begin(keyMap), std::end(keyMap)}, map{map}, devSubtype{devSubtype} {}
	constexpr KeyConfig(Input::Map map, std::string_view name, auto &&keyMap):
		KeyConfig{map, {}, name, IG_forward(keyMap)} {}
	constexpr KeyConfig(KeyConfigDesc desc):
		KeyConfig{desc.map, desc.devSubtype, desc.name, desc.keyMap} {}

	constexpr bool operator==(KeyConfig const& rhs) const { return name == rhs.name; }
	constexpr bool operator==(KeyConfigDesc const& rhs) const { return name == rhs.name; }
	constexpr bool operator==(std::string_view rhs) const { return name == rhs; }
	constexpr explicit operator bool() { return name.size(); }
	constexpr KeyConfigDesc desc() const { return{map, devSubtype, name, keyMap}; }
	void set(KeyInfo, MappedKeys);
	void unbindCategory(const KeyCategory &category);
	void resetCategory(const KeyCategory &category, KeyConfigDesc defaultConf);
	constexpr auto find(KeyInfo key) { return std::ranges::find_if(keyMap, [&](auto &val){ return val.key == key; }); }
	constexpr MappedKeys get(KeyInfo key) const { return desc().get(key); }
	static KeyConfig readConfig(MapIO &);
	void writeConfig(FileIO &) const;

private:
	std::vector<KeyMapping> keyMap;
	Input::Map map{};
	Input::DeviceSubtype devSubtype{};
};

struct AxisAsDpadFlags
{
	uint8_t
	stick1:1{},
	unused1:1{},
	stick2:1{},
	triggers:1{},
	pedals:1{},
	unused2:1{},
	hat:1{};

	constexpr bool operator==(AxisAsDpadFlags const&) const = default;
};

struct InputDeviceSavedConfig
{
	static constexpr uint8_t ENUM_ID_MASK = 0x1F;
	static constexpr uint8_t HANDLE_UNBOUND_EVENTS_FLAG = 0x80;

	std::string keyConfName;
	std::string name;
	uint8_t enumId{};
	int8_t player{};
	bool enabled = true;
	AxisAsDpadFlags joystickAxisAsDpadFlags;
	ConditionalMember<hasICadeInput, bool> iCadeMode{};
	ConditionalMember<Config::envIsAndroid, bool> handleUnboundEvents{};

	constexpr bool operator ==(InputDeviceSavedConfig const& rhs) const
	{
		return enumId == rhs.enumId && name == rhs.name;
	}

	bool matchesDevice(const Input::Device& dev) const;
};

struct InputDeviceSavedSessionConfig
{
	std::string keyConfName;
	std::string name;
	uint8_t enumId{};
	int8_t player{};

	constexpr bool operator ==(InputDeviceSavedSessionConfig const& rhs) const
	{
		return enumId == rhs.enumId && name == rhs.name;
	}

	bool matchesDevice(const Input::Device& dev) const;
};

struct SystemKeyInputFlags
{
	bool allowTurboModifier{true};
};

}
