/*  This file is part of Snes9x EX.

	Please see COPYING file in root directory for license information. */

#include <emuframework/macros.h>
import system;
import emuex;
import imagine;
import std;
#include <emuframework/EmuAppInlines.hh>

namespace EmuEx
{

const std::string_view AppMeta::creditsViewStr{CREDITS_INFO_STRING "(c) 2011-2026\nRobert Broglia\nwww.explusalpha.com\n\nPortions (c) the\nSnes9x Team\nwww.snes9x.com"};
#ifdef SNES9X_VERSION_1_4
const std::string_view AppMeta::configFilename{"Snes9x.config"};
#else
const std::string_view AppMeta::configFilename{"Snes9xP.config"};
constexpr BundledGameInfo gameInfo{"Bio Worm", Config::envIsLinux ? "BioWorm.7z" : "Bio Worm.7z"};
const std::span<const BundledGameInfo> AppMeta::bundledGameInfo{&gameInfo, 1};
#endif
const bool AppMeta::hasCheats{true};
const bool AppMeta::hasPALVideoSystem{true};
const bool AppMeta::hasResetModes{true};
const bool AppMeta::canRenderRGBA8888{};
const bool AppMeta::hasRectangularPixels{true};
const int AppMeta::maxPlayers{5};
const bool AppMeta::needsGlobalInstance{true};
const NameFilterFunc AppMeta::defaultFsFilter = [](std::string_view name)
{
	return endsWithAnyCaseless(name, ".smc", ".sfc", ".swc", ".bs", ".st", ".fig", ".mgd");
};

constexpr auto dpadKeyInfo = makeArray<KeyInfo>
(
	SnesKey::Up,
	SnesKey::Right,
	SnesKey::Down,
	SnesKey::Left
);

constexpr auto centerKeyInfo = makeArray<KeyInfo>
(
	SnesKey::Select,
	SnesKey::Start
);

constexpr auto faceKeyInfo = makeArray<KeyInfo>
(
	SnesKey::B,
	SnesKey::A,
	SnesKey::Y,
	SnesKey::X
);

constexpr auto turboFaceKeyInfo = turbo(faceKeyInfo);

constexpr auto faceLRKeyInfo = makeArray<KeyInfo>
(
	SnesKey::B,
	SnesKey::A,
	SnesKey::R,
	SnesKey::Y,
	SnesKey::X,
	SnesKey::L
);

constexpr auto lKeyInfo = makeArray<KeyInfo>(SnesKey::L);
constexpr auto rKeyInfo = makeArray<KeyInfo>(SnesKey::R);

constexpr auto gpKeyInfo = concatToArrayNow<dpadKeyInfo, centerKeyInfo, faceLRKeyInfo, turboFaceKeyInfo>;
constexpr auto gp2KeyInfo = transpose(gpKeyInfo, 1);
constexpr auto gp3KeyInfo = transpose(gpKeyInfo, 2);
constexpr auto gp4KeyInfo = transpose(gpKeyInfo, 3);
constexpr auto gp5KeyInfo = transpose(gpKeyInfo, 4);

std::span<const KeyCategory> AppMeta::keyCategories()
{
	static constexpr std::array categories
	{
		KeyCategory{"Gamepad", gpKeyInfo},
		KeyCategory{"Gamepad 2", gp2KeyInfo, 1},
		KeyCategory{"Gamepad 3", gp3KeyInfo, 2},
		KeyCategory{"Gamepad 4", gp4KeyInfo, 3},
		KeyCategory{"Gamepad 5", gp5KeyInfo, 4},
	};
	return categories;
}

std::string_view AppMeta::systemKeyCodeToString(KeyCode c)
{
	switch(SnesKey(c))
	{
		case SnesKey::Up: return "Up";
		case SnesKey::Right: return "Right";
		case SnesKey::Down: return "Down";
		case SnesKey::Left: return "Left";
		case SnesKey::Select: return "Select";
		case SnesKey::Start: return "Start";
		case SnesKey::A: return "A";
		case SnesKey::B: return "B";
		case SnesKey::X: return "X";
		case SnesKey::Y: return "Y";
		case SnesKey::L: return "L";
		case SnesKey::R: return "R";
		default: return "";
	}
}

std::span<const KeyConfigDesc> AppMeta::defaultKeyConfigs()
{
	using namespace Input;

	static constexpr std::array pcKeyboardMap
	{
		KeyMapping{SnesKey::Up, Keycode::UP},
		KeyMapping{SnesKey::Right, Keycode::RIGHT},
		KeyMapping{SnesKey::Down, Keycode::DOWN},
		KeyMapping{SnesKey::Left, Keycode::LEFT},
		KeyMapping{SnesKey::Select, Keycode::SPACE},
		KeyMapping{SnesKey::Start, Keycode::ENTER},
		KeyMapping{SnesKey::B, Keycode::Z},
		KeyMapping{SnesKey::A, Keycode::X},
		KeyMapping{SnesKey::Y, Keycode::A},
		KeyMapping{SnesKey::X, Keycode::S},
		KeyMapping{SnesKey::L, Keycode::Q},
		KeyMapping{SnesKey::R, Keycode::W},
	};

	static constexpr std::array genericGamepadMap
	{
		KeyMapping{SnesKey::Up, Keycode::UP},
		KeyMapping{SnesKey::Right, Keycode::RIGHT},
		KeyMapping{SnesKey::Down, Keycode::DOWN},
		KeyMapping{SnesKey::Left, Keycode::LEFT},
		KeyMapping{SnesKey::Select, Keycode::GAME_SELECT},
		KeyMapping{SnesKey::Start, Keycode::GAME_START},
		KeyMapping{SnesKey::B, Keycode::GAME_A},
		KeyMapping{SnesKey::A, Keycode::GAME_B},
		KeyMapping{SnesKey::Y, Keycode::GAME_X},
		KeyMapping{SnesKey::X, Keycode::GAME_Y},
		KeyMapping{SnesKey::L, Keycode::GAME_L1},
		KeyMapping{SnesKey::R, Keycode::GAME_R1},
	};

	static constexpr std::array wiimoteMap
	{
		KeyMapping{SnesKey::Up, WiimoteKey::UP},
		KeyMapping{SnesKey::Right, WiimoteKey::RIGHT},
		KeyMapping{SnesKey::Down, WiimoteKey::DOWN},
		KeyMapping{SnesKey::Left, WiimoteKey::LEFT},
		KeyMapping{SnesKey::B, WiimoteKey::_1},
		KeyMapping{SnesKey::A, WiimoteKey::_2},
		KeyMapping{SnesKey::Y, WiimoteKey::A},
		KeyMapping{SnesKey::X, WiimoteKey::B},
		KeyMapping{SnesKey::Select, WiimoteKey::MINUS},
		KeyMapping{SnesKey::Start, WiimoteKey::PLUS},
	};

	return genericKeyConfigs<pcKeyboardMap, genericGamepadMap, wiimoteMap>();
}

bool AppMeta::allowsTurboModifier(KeyCode c)
{
	switch(SnesKey(c))
	{
		case SnesKey::R ... SnesKey::A:
		case SnesKey::Y ... SnesKey::B:
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
		x{AssetFileID::gamepadOverlay,      gpImageCoords({{4, 2}, {2, 2}})},
		y{AssetFileID::gamepadOverlay,      gpImageCoords({{6, 2}, {2, 2}})},
		l{AssetFileID::gamepadOverlay,      gpImageCoords({{4, 4}, {2, 2}})},
		r{AssetFileID::gamepadOverlay,      gpImageCoords({{6, 4}, {2, 2}})},
		select{AssetFileID::gamepadOverlay, gpImageCoords({{0, 6}, {2, 1}}), {1, 2}},
		start{AssetFileID::gamepadOverlay,  gpImageCoords({{0, 7}, {2, 1}}), {1, 2}},

		blank{AssetFileID::gamepadOverlay, gpImageCoords({{0, 4}, {2, 2}})};
	} virtualControllerAssets;

	if(key[0] == 0)
		return virtualControllerAssets.dpad;
	switch(SnesKey(key[0]))
	{
		case SnesKey::A: return virtualControllerAssets.a;
		case SnesKey::B: return virtualControllerAssets.b;
		case SnesKey::X: return virtualControllerAssets.x;
		case SnesKey::Y: return virtualControllerAssets.y;
		case SnesKey::L: return virtualControllerAssets.l;
		case SnesKey::R: return virtualControllerAssets.r;
		case SnesKey::Select: return virtualControllerAssets.select;
		case SnesKey::Start: return virtualControllerAssets.start;
		default: return virtualControllerAssets.blank;
	}
}

SystemInputDeviceDesc AppMeta::inputDeviceDesc(int)
{
	static constexpr std::array gamepadComponents
	{
		InputComponentDesc{"D-Pad", dpadKeyInfo, InputComponent::dPad, LB2DO},
		InputComponentDesc{"Face Buttons", faceKeyInfo, InputComponent::button, RB2DO, {.staggeredLayout = true}},
		InputComponentDesc{"Face Buttons + Inline L/R", faceLRKeyInfo, InputComponent::button, RB2DO, {.altConfig = true, .staggeredLayout = true}},
		InputComponentDesc{"L", lKeyInfo, InputComponent::trigger, LB2DO},
		InputComponentDesc{"R", rKeyInfo, InputComponent::trigger, RB2DO},
		InputComponentDesc{"Select", {&centerKeyInfo[0], 1}, InputComponent::button, LB2DO},
		InputComponentDesc{"Start", {&centerKeyInfo[1], 1}, InputComponent::button, RB2DO},
		InputComponentDesc{"Select/Start", centerKeyInfo, InputComponent::button, CB2DO, {.altConfig = true}},
	};
	static constexpr SystemInputDeviceDesc gamepadDesc{"Gamepad", gamepadComponents};
	return gamepadDesc;
}

void AppMeta::onCustomizeNavView(AppNavView& view)
{
	const Gfx::LGradientStopDesc navViewGrad[] =
	{
		{ .0, Gfx::PackedColor::format.build((139./255.) * .4, (149./255.) * .4, (230./255.) * .4, 1.) },
		{ .3, Gfx::PackedColor::format.build((139./255.) * .4, (149./255.) * .4, (230./255.) * .4, 1.) },
		{ .97, Gfx::PackedColor::format.build((46./255.) * .4, (50./255.) * .4, (77./255.) * .4, 1.) },
		{ 1., view.separatorColor() },
	};
	view.setBackgroundGradient(navViewGrad);
}

}
