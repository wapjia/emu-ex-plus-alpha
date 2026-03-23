/*  This file is part of NES.emu.

	NES.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	NES.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with NES.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/macros.h>
import system;
import emuex;
import imagine;
import std;
#include <emuframework/EmuAppInlines.hh>

namespace EmuEx
{

const std::string_view AppMeta::creditsViewStr{CREDITS_INFO_STRING "(c) 2011-2026\nRobert Broglia\nwww.explusalpha.com\n\nPortions (c) the\nFCEUX Team\nfceux.com"};
const std::string_view AppMeta::configFilename{"NesEmu.config"};
const bool AppMeta::hasCheats{true};
const bool AppMeta::hasPALVideoSystem{true};
const bool AppMeta::hasResetModes{true};
const bool AppMeta::hasRectangularPixels{true};
const int AppMeta::maxPlayers{4};
const bool AppMeta::needsGlobalInstance{true};
const NameFilterFunc AppMeta::defaultFsFilter{hasNESExtension};

constexpr auto dpadKeyInfo = makeArray<KeyInfo>
(
	NesKey::Up,
	NesKey::Right,
	NesKey::Down,
	NesKey::Left
);

constexpr auto centerKeyInfo = makeArray<KeyInfo>
(
	NesKey::Select,
	NesKey::Start
);

constexpr std::array p2StartKeyInfo
{
	KeyInfo{NesKey::Start, {.deviceId = 1}}
};

constexpr auto faceKeyInfo = makeArray<KeyInfo>
(
	NesKey::B,
	NesKey::A
);

constexpr auto turboFaceKeyInfo = turbo(faceKeyInfo);

constexpr std::array comboKeyInfo{KeyInfo{std::array{NesKey::A, NesKey::B}}};

constexpr auto exKeyInfo = makeArray<KeyInfo>
(
	NesKey::toggleDiskSide
);

constexpr auto gpKeyInfo = concatToArrayNow<dpadKeyInfo, centerKeyInfo, faceKeyInfo, turboFaceKeyInfo, comboKeyInfo>;
constexpr auto gp2KeyInfo = transpose(gpKeyInfo, 1);
constexpr auto gp3KeyInfo = transpose(gpKeyInfo, 2);
constexpr auto gp4KeyInfo = transpose(gpKeyInfo, 3);

std::span<const KeyCategory> AppMeta::keyCategories()
{
	static constexpr std::array categories
	{
		KeyCategory{"Gamepad", gpKeyInfo},
		KeyCategory{"Gamepad 2", gp2KeyInfo, 1},
		KeyCategory{"Gamepad 3", gp3KeyInfo, 2},
		KeyCategory{"Gamepad 4", gp4KeyInfo, 3},
		KeyCategory{"Extra Functions", exKeyInfo},
	};
	return categories;
}

std::string_view AppMeta::systemKeyCodeToString(KeyCode c)
{
	switch(NesKey(c))
	{
		case NesKey::Up: return "Up";
		case NesKey::Right: return "Right";
		case NesKey::Down: return "Down";
		case NesKey::Left: return "Left";
		case NesKey::Select: return "Select";
		case NesKey::Start: return "Start";
		case NesKey::A: return "A";
		case NesKey::B: return "B";
		case NesKey::toggleDiskSide: return "Eject Disk/Switch Side";
		default: return "";
	}
}

std::span<const KeyConfigDesc> AppMeta::defaultKeyConfigs()
{
	using namespace Input;

	static constexpr std::array pcKeyboardMap
	{
		KeyMapping{NesKey::Up, Keycode::UP},
		KeyMapping{NesKey::Right, Keycode::RIGHT},
		KeyMapping{NesKey::Down, Keycode::DOWN},
		KeyMapping{NesKey::Left, Keycode::LEFT},
		KeyMapping{NesKey::Select, Keycode::SPACE},
		KeyMapping{NesKey::Start, Keycode::ENTER},
		KeyMapping{NesKey::B, Keycode::Z},
		KeyMapping{NesKey::A, Keycode::X},
	};

	static constexpr std::array genericGamepadMap
	{
		KeyMapping{NesKey::Up, Keycode::UP},
		KeyMapping{NesKey::Right, Keycode::RIGHT},
		KeyMapping{NesKey::Down, Keycode::DOWN},
		KeyMapping{NesKey::Left, Keycode::LEFT},
		KeyMapping{NesKey::Select, Keycode::GAME_SELECT},
		KeyMapping{NesKey::Start, Keycode::GAME_START},
		KeyMapping{NesKey::B, Keycode::GAME_X},
		KeyMapping{NesKey::A, Keycode::GAME_A},
	};

	static constexpr std::array wiimoteMap
	{
		KeyMapping{NesKey::Up, WiimoteKey::UP},
		KeyMapping{NesKey::Right, WiimoteKey::RIGHT},
		KeyMapping{NesKey::Down, WiimoteKey::DOWN},
		KeyMapping{NesKey::Left, WiimoteKey::LEFT},
		KeyMapping{NesKey::B, WiimoteKey::_1},
		KeyMapping{NesKey::A, WiimoteKey::_2},
		KeyMapping{NesKey::Select, WiimoteKey::MINUS},
		KeyMapping{NesKey::Start, WiimoteKey::PLUS},
	};

	return genericKeyConfigs<pcKeyboardMap, genericGamepadMap, wiimoteMap>();
}

bool AppMeta::allowsTurboModifier(KeyCode c)
{
	switch(NesKey(c))
	{
		case NesKey::A ... NesKey::B:
			return true;
		default: return false;
	}
}

constexpr FRect gpImageCoords(IRect cellRelBounds)
{
	constexpr F2Size imageSize{256, 256};
	constexpr int cellSize = 32;
	return (cellRelBounds.relToAbs() * cellSize).as<float>() / imageSize;
}

AssetDesc AppMeta::vControllerAssetDesc(KeyInfo key)
{
	static constexpr struct VirtualControllerAssets
	{
		AssetDesc dpad{AssetFileID::gamepadOverlay, gpImageCoords({{}, {4, 4}})},

		a{AssetFileID::gamepadOverlay,      gpImageCoords({{4, 0}, {2, 2}})},
		b{AssetFileID::gamepadOverlay,      gpImageCoords({{6, 0}, {2, 2}})},
		select{AssetFileID::gamepadOverlay, gpImageCoords({{4, 2}, {2, 1}}), {1, 2}},
		start{AssetFileID::gamepadOverlay,  gpImageCoords({{4, 3}, {2, 1}}), {1, 2}},
		ab{AssetFileID::gamepadOverlay,     gpImageCoords({{6, 2}, {2, 2}})},

		blank{AssetFileID::gamepadOverlay, gpImageCoords({{0, 4}, {2, 2}})};
	} virtualControllerAssets;

	if(key[0] == 0)
		return virtualControllerAssets.dpad;
	switch(NesKey(key[0]))
	{
		case NesKey::A: return NesKey(key[1]) == NesKey::B ? virtualControllerAssets.ab : virtualControllerAssets.a;
		case NesKey::B: return virtualControllerAssets.b;
		case NesKey::Select: return virtualControllerAssets.select;
		case NesKey::Start: return virtualControllerAssets.start;
		default: return virtualControllerAssets.blank;
	}
}

SystemInputDeviceDesc AppMeta::inputDeviceDesc(int idx)
{
	static constexpr std::array gamepadComponents
	{
		InputComponentDesc{"D-Pad", dpadKeyInfo, InputComponent::dPad, LB2DO},
		InputComponentDesc{"Face Buttons", faceKeyInfo, InputComponent::button, RB2DO},
		InputComponentDesc{"Select", {&centerKeyInfo[0], 1}, InputComponent::button, LB2DO},
		InputComponentDesc{"Start", {&centerKeyInfo[1], 1}, InputComponent::button, RB2DO},
		InputComponentDesc{"Select/Start", centerKeyInfo, InputComponent::button, CB2DO, {.altConfig = true}},
		InputComponentDesc{"P2 Start (Famicom Microphone)", p2StartKeyInfo, InputComponent::button, RB2DO, {.altConfig = true}},
	};
	static constexpr SystemInputDeviceDesc gamepadDesc{"Gamepad", gamepadComponents};
	return gamepadDesc;
}

void AppMeta::onCustomizeNavView(AppNavView& view)
{
	const Gfx::LGradientStopDesc navViewGrad[] =
	{
		{ .0, Gfx::PackedColor::format.build(1. * .4, 0., 0., 1.) },
		{ .3, Gfx::PackedColor::format.build(1. * .4, 0., 0., 1.) },
		{ .97, Gfx::PackedColor::format.build(.5 * .4, 0., 0., 1.) },
		{ 1., view.separatorColor() },
	};
	view.setBackgroundGradient(navViewGrad);
}

}
