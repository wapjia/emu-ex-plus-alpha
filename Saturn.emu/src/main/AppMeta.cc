/*  This file is part of Saturn.emu.

	Saturn.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Saturn.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Saturn.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/macros.h>
#include <mednafen/types.h>
import system;
import emuex;
import imagine;
import std;
#include <emuframework/EmuAppInlines.hh>

namespace EmuEx
{

const std::string_view AppMeta::creditsViewStr{CREDITS_INFO_STRING "(c) 2011-2026\nRobert Broglia\nwww.explusalpha.com\n\nPortions (c) the\nMednafen Team\nmednafen.github.io"};
const std::string_view AppMeta::configFilename{"SaturnEmu.config"};
const bool AppMeta::handlesArchiveFiles{true};
const bool AppMeta::hasResetModes{true};
const bool AppMeta::hasRectangularPixels{true};
const bool AppMeta::hasPALVideoSystem{true};
const bool AppMeta::canRenderRGB565{};
const bool AppMeta::stateSizeChangesAtRuntime{true};
const int AppMeta::maxPlayers{12};
const bool AppMeta::needsGlobalInstance{true};
const NameFilterFunc AppMeta::defaultFsFilter{hasCDExtension};

constexpr auto dpadKeyInfo = makeArray<KeyInfo>
(
	SaturnKey::Up,
	SaturnKey::Right,
	SaturnKey::Down,
	SaturnKey::Left
);

constexpr auto centerKeyInfo = makeArray<KeyInfo>
(
	SaturnKey::Start
);

constexpr auto faceKeyInfo = makeArray<KeyInfo>
(
	SaturnKey::A,
	SaturnKey::B,
	SaturnKey::C,
	SaturnKey::X,
	SaturnKey::Y,
	SaturnKey::Z
);

constexpr auto faceLRKeyInfo = makeArray<KeyInfo>
(
	SaturnKey::A,
	SaturnKey::B,
	SaturnKey::C,
	SaturnKey::X,
	SaturnKey::Y,
	SaturnKey::Z,
	SaturnKey::L,
	SaturnKey::R
);

constexpr auto turboFaceKeyInfo = turbo(faceLRKeyInfo);

constexpr auto lKeyInfo = makeArray<KeyInfo>(SaturnKey::L);
constexpr auto rKeyInfo = makeArray<KeyInfo>(SaturnKey::R);

constexpr auto gpKeyInfo = concatToArrayNow<dpadKeyInfo, centerKeyInfo, faceLRKeyInfo, turboFaceKeyInfo>;
constexpr auto gp2KeyInfo = transpose(gpKeyInfo, 1);
constexpr auto gp3KeyInfo = transpose(gpKeyInfo, 2);
constexpr auto gp4KeyInfo = transpose(gpKeyInfo, 3);
constexpr auto gp5KeyInfo = transpose(gpKeyInfo, 4);
constexpr auto gp6KeyInfo = transpose(gpKeyInfo, 5);
constexpr auto gp7KeyInfo = transpose(gpKeyInfo, 6);
constexpr auto gp8KeyInfo = transpose(gpKeyInfo, 7);
constexpr auto gp9KeyInfo = transpose(gpKeyInfo, 8);
constexpr auto gp10KeyInfo = transpose(gpKeyInfo, 9);
constexpr auto gp11KeyInfo = transpose(gpKeyInfo, 10);
constexpr auto gp12KeyInfo = transpose(gpKeyInfo, 11);

std::span<const KeyCategory> AppMeta::keyCategories()
{
	static constexpr std::array categories
	{
		KeyCategory{"Gamepad",    gpKeyInfo},
		KeyCategory{"Gamepad 2",  gp2KeyInfo, 1},
		KeyCategory{"Gamepad 3",  gp3KeyInfo, 2},
		KeyCategory{"Gamepad 4",  gp4KeyInfo, 3},
		KeyCategory{"Gamepad 5",  gp5KeyInfo, 4},
		KeyCategory{"Gamepad 6",  gp6KeyInfo, 5},
		KeyCategory{"Gamepad 7",  gp7KeyInfo, 6},
		KeyCategory{"Gamepad 8",  gp8KeyInfo, 7},
		KeyCategory{"Gamepad 9",  gp9KeyInfo, 8},
		KeyCategory{"Gamepad 10", gp10KeyInfo, 9},
		KeyCategory{"Gamepad 11", gp11KeyInfo, 10},
		KeyCategory{"Gamepad 12", gp12KeyInfo, 11},
	};
	return categories;
}

std::string_view AppMeta::systemKeyCodeToString(KeyCode c)
{
	switch(SaturnKey(c))
	{
		case SaturnKey::Up: return "Up";
		case SaturnKey::Right: return "Right";
		case SaturnKey::Down: return "Down";
		case SaturnKey::Left: return "Left";
		case SaturnKey::Start: return "Start";
		case SaturnKey::A: return "A";
		case SaturnKey::B: return "B";
		case SaturnKey::C: return "C";
		case SaturnKey::X: return "X";
		case SaturnKey::Y: return "Y";
		case SaturnKey::Z: return "Z";
		case SaturnKey::L: return "L";
		case SaturnKey::R: return "R";
		default: return "";
	}
}

std::span<const KeyConfigDesc> AppMeta::defaultKeyConfigs()
{
	using namespace Input;

	static constexpr std::array pcKeyboardMap
	{
		KeyMapping{SaturnKey::Up, Keycode::UP},
		KeyMapping{SaturnKey::Right, Keycode::RIGHT},
		KeyMapping{SaturnKey::Down, Keycode::DOWN},
		KeyMapping{SaturnKey::Left, Keycode::LEFT},
		KeyMapping{SaturnKey::Start, Keycode::ENTER},
		KeyMapping{SaturnKey::A, Keycode::Z},
		KeyMapping{SaturnKey::B, Keycode::X},
		KeyMapping{SaturnKey::C, Keycode::C},
		KeyMapping{SaturnKey::X, Keycode::A},
		KeyMapping{SaturnKey::Y, Keycode::S},
		KeyMapping{SaturnKey::Z, Keycode::D},
		KeyMapping{SaturnKey::L, Keycode::Q},
		KeyMapping{SaturnKey::R, Keycode::E},
	};

	static constexpr std::array genericGamepadMap
	{
		KeyMapping{SaturnKey::Up, Keycode::UP},
		KeyMapping{SaturnKey::Right, Keycode::RIGHT},
		KeyMapping{SaturnKey::Down, Keycode::DOWN},
		KeyMapping{SaturnKey::Left, Keycode::LEFT},
		KeyMapping{SaturnKey::Start, Keycode::GAME_START},
		KeyMapping{SaturnKey::A, Keycode::GAME_X},
		KeyMapping{SaturnKey::B, Keycode::GAME_A},
		KeyMapping{SaturnKey::C, Keycode::GAME_B},
		KeyMapping{SaturnKey::X, Keycode::GAME_L1},
		KeyMapping{SaturnKey::Y, Keycode::GAME_Y},
		KeyMapping{SaturnKey::Z, Keycode::GAME_R1},
		KeyMapping{SaturnKey::L, Keycode::GAME_L2},
		KeyMapping{SaturnKey::R, Keycode::GAME_R2},
	};

	static constexpr std::array wiimoteMap
	{
		KeyMapping{SaturnKey::Up, WiimoteKey::UP},
		KeyMapping{SaturnKey::Right, WiimoteKey::RIGHT},
		KeyMapping{SaturnKey::Down, WiimoteKey::DOWN},
		KeyMapping{SaturnKey::Left, WiimoteKey::LEFT},
		KeyMapping{SaturnKey::A, WiimoteKey::_1},
		KeyMapping{SaturnKey::B, WiimoteKey::_2},
		KeyMapping{SaturnKey::C, WiimoteKey::A},
		KeyMapping{SaturnKey::X, WiimoteKey::B},
		KeyMapping{SaturnKey::Y, WiimoteKey::MINUS},
		KeyMapping{SaturnKey::Start, WiimoteKey::PLUS},
	};

	return genericKeyConfigs<pcKeyboardMap, genericGamepadMap, wiimoteMap, genericGamepadModifierAppKeyCodeMap>();
}

bool AppMeta::allowsTurboModifier(KeyCode c)
{
	switch(SaturnKey(c))
	{
		case SaturnKey::Z ... SaturnKey::R:
		case SaturnKey::B ... SaturnKey::A:
		case SaturnKey::L:
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
	constexpr struct VirtualControllerAssets
	{
		AssetDesc dpad{AssetFileID::gamepadOverlay, gpImageCoords({{}, {4, 4}})},

		a{AssetFileID::gamepadOverlay,     gpImageCoords({{4, 0}, {2, 2}})},
		b{AssetFileID::gamepadOverlay,     gpImageCoords({{6, 0}, {2, 2}})},
		c{AssetFileID::gamepadOverlay,     gpImageCoords({{4, 2}, {2, 2}})},
		x{AssetFileID::gamepadOverlay,     gpImageCoords({{6, 2}, {2, 2}})},
		y{AssetFileID::gamepadOverlay,     gpImageCoords({{0, 4}, {2, 2}})},
		z{AssetFileID::gamepadOverlay,     gpImageCoords({{2, 4}, {2, 2}})},
		l{AssetFileID::gamepadOverlay,     gpImageCoords({{4, 4}, {2, 2}})},
		r{AssetFileID::gamepadOverlay,     gpImageCoords({{6, 4}, {2, 2}})},
		start{AssetFileID::gamepadOverlay, gpImageCoords({{0, 6}, {2, 1}}), {1, 2}},
		blank{AssetFileID::gamepadOverlay, gpImageCoords({{2, 6}, {2, 2}})};
	} virtualControllerAssets;

	if(key[0] == 0)
		return virtualControllerAssets.dpad;
	switch(SaturnKey(key[0]))
	{
		case SaturnKey::A: return virtualControllerAssets.a;
		case SaturnKey::B: return virtualControllerAssets.b;
		case SaturnKey::C: return virtualControllerAssets.c;
		case SaturnKey::X: return virtualControllerAssets.x;
		case SaturnKey::Y: return virtualControllerAssets.y;
		case SaturnKey::Z: return virtualControllerAssets.z;
		case SaturnKey::L: return virtualControllerAssets.l;
		case SaturnKey::R: return virtualControllerAssets.r;
		case SaturnKey::Start: return virtualControllerAssets.start;
		default: return virtualControllerAssets.blank;
	}
}

SystemInputDeviceDesc AppMeta::inputDeviceDesc(int idx)
{
	static constexpr std::array gamepadComponents
	{
		InputComponentDesc{"D-Pad", dpadKeyInfo, InputComponent::dPad, LB2DO},
		InputComponentDesc{"Face Buttons", faceKeyInfo, InputComponent::button, RB2DO},
		InputComponentDesc{"Face Buttons + Inline L/R", faceLRKeyInfo, InputComponent::button, RB2DO, {.altConfig = true}},
		InputComponentDesc{"L", lKeyInfo, InputComponent::trigger, LB2DO},
		InputComponentDesc{"R", rKeyInfo, InputComponent::trigger, RB2DO},
		InputComponentDesc{"Start", centerKeyInfo, InputComponent::button, RB2DO},
	};
	static constexpr SystemInputDeviceDesc gamepadDesc{"Gamepad", gamepadComponents};
	return gamepadDesc;
}

void AppMeta::onCustomizeNavView(AppNavView& view)
{
	const Gfx::LGradientStopDesc navViewGrad[] =
	{
		{ .0, Gfx::PackedColor::format.build((103./255.) * .7, (176./255.) * .7, (255./255.) * .7, 1.) },
		{ .3, Gfx::PackedColor::format.build((103./255.) * .7, (176./255.) * .7, (255./255.) * .7, 1.) },
		{ .97, Gfx::PackedColor::format.build((103./255.) * .4, (176./255.) * .4, (255./255.) * .4, 1.) },
		{ 1., view.separatorColor() },
	};
	view.setBackgroundGradient(navViewGrad);
}

}
