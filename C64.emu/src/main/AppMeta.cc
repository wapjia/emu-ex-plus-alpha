/*  This file is part of C64.emu.

	C64.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	C64.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with C64.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/macros.h>
import system;
import emuex;
import imagine;
import std;
#include <emuframework/EmuAppInlines.hh>

namespace EmuEx
{

const std::string_view AppMeta::creditsViewStr{CREDITS_INFO_STRING "(c) 2013-2026\nRobert Broglia\nwww.explusalpha.com\n\nPortions (c) the\nVice Team\nvice-emu.sourceforge.io"};
const std::string_view AppMeta::configFilename{"C64Emu.config"};
const bool AppMeta::hasPALVideoSystem{true};
const bool AppMeta::hasResetModes{true};
const bool AppMeta::handlesGenericIO{};
const bool AppMeta::hasRectangularPixels{true};
const bool AppMeta::stateSizeChangesAtRuntime{true};
const bool AppMeta::inputHasKeyboard{true};
const int AppMeta::maxPlayers{2};
const bool AppMeta::needsGlobalInstance{true};
const bool AppMeta::handlesRecentContent{true};
const NameFilterFunc AppMeta::defaultFsFilter{hasC64Extension};

constexpr auto specialFunctionKeyInfo = makeArray<KeyInfo>
(
	C64Key::SwapJSPorts,
	C64Key::ToggleKB
);

constexpr auto dpadKeyInfo = makeArray<KeyInfo>
(
	C64Key::Up,
	C64Key::Right,
	C64Key::Down,
	C64Key::Left
);

constexpr auto triggerKeyInfo = makeArray<KeyInfo>(C64Key::JSTrigger);
constexpr auto turboTriggerKeyInfo = turbo(triggerKeyInfo);

constexpr auto shortcutKeyInfo = makeArray<KeyInfo>
(
	C64Key::KeyboardF1,
	C64Key::ToggleKB
);

constexpr auto kbKeyInfo = makeArray<KeyInfo>
(
	C64Key::KeyboardF1,
	C64Key::KeyboardF2,
	C64Key::KeyboardF3,
	C64Key::KeyboardF4,
	C64Key::KeyboardF5,
	C64Key::KeyboardF6,
	C64Key::KeyboardF7,
	C64Key::KeyboardF8,
	C64Key::Keyboard1,
	C64Key::Keyboard2,
	C64Key::Keyboard3,
	C64Key::Keyboard4,
	C64Key::Keyboard5,
	C64Key::Keyboard6,
	C64Key::Keyboard7,
	C64Key::Keyboard8,
	C64Key::Keyboard9,
	C64Key::Keyboard0,
	C64Key::KeyboardExclam,
	C64Key::KeyboardAt,
	C64Key::KeyboardNumberSign,
	C64Key::KeyboardDollar,
	C64Key::KeyboardPercent,
	C64Key::KeyboardAmpersand,
	C64Key::KeyboardAsterisk,
	C64Key::KeyboardParenLeft,
	C64Key::KeyboardParenRight,
	C64Key::KeyboardPlus,
	C64Key::KeyboardMinus,
	C64Key::KeyboardLeftArrow,
	C64Key::KeyboardPound,
	C64Key::KeyboardClrHome,
	C64Key::KeyboardInstDel,
	C64Key::KeyboardCtrl,
	C64Key::KeyboardQ,
	C64Key::KeyboardW,
	C64Key::KeyboardE,
	C64Key::KeyboardR,
	C64Key::KeyboardT,
	C64Key::KeyboardY,
	C64Key::KeyboardU,
	C64Key::KeyboardI,
	C64Key::KeyboardO,
	C64Key::KeyboardP,
	C64Key::KeyboardUpArrow,
	C64Key::KeyboardRestore,
	C64Key::KeyboardRunStop,
	C64Key::KeyboardShiftLock,
	C64Key::KeyboardA,
	C64Key::KeyboardS,
	C64Key::KeyboardD,
	C64Key::KeyboardF,
	C64Key::KeyboardG,
	C64Key::KeyboardH,
	C64Key::KeyboardJ,
	C64Key::KeyboardK,
	C64Key::KeyboardL,
	C64Key::KeyboardSemiColon,
	C64Key::KeyboardColon,
	C64Key::KeyboardEquals,
	C64Key::KeyboardReturn,
	C64Key::KeyboardCommodore,
	C64Key::KeyboardLeftShift,
	C64Key::KeyboardZ,
	C64Key::KeyboardX,
	C64Key::KeyboardC,
	C64Key::KeyboardV,
	C64Key::KeyboardB,
	C64Key::KeyboardN,
	C64Key::KeyboardM,
	C64Key::KeyboardComma,
	C64Key::KeyboardPeriod,
	C64Key::KeyboardSlash,
	C64Key::KeyboardApostrophe,
	C64Key::KeyboardLess,
	C64Key::KeyboardGreater,
	C64Key::KeyboardQuestion,
	C64Key::KeyboardQuoteDbl,
	C64Key::KeyboardRightShift,
	C64Key::KeyboardUp,
	C64Key::KeyboardRight,
	C64Key::KeyboardDown,
	C64Key::KeyboardLeft,
	C64Key::KeyboardSpace,
	C64Key::KeyboardCtrlLock,
	C64Key::KeyboardBracketLeft,
	C64Key::KeyboardBracketRight
);

constexpr auto jsKeyInfo = concatToArrayNow<dpadKeyInfo, triggerKeyInfo, turboTriggerKeyInfo>;
constexpr auto js2KeyInfo = transpose(jsKeyInfo, 1);

std::span<const KeyCategory> AppMeta::keyCategories()
{
	static constexpr KeyCategory categories[]
	{
		{"Joystick", jsKeyInfo},
		{"Joystick 2", js2KeyInfo, 1},
		{"Special Functions", specialFunctionKeyInfo},
		{"Keyboard", kbKeyInfo},
	};
	return categories;
}

std::string_view AppMeta::systemKeyCodeToString(KeyCode c)
{
	switch(C64Key(c))
	{
		case C64Key::Up: return "Up";
		case C64Key::Right: return "Right";
		case C64Key::Down: return "Down";
		case C64Key::Left: return "Left";
		case C64Key::JSTrigger: return "Trigger";
		case C64Key::SwapJSPorts: return "Swap Ports";
		case C64Key::ToggleKB: return "Toggle Keyboard";
		case C64Key::KeyboardF1: return "F1";
		case C64Key::KeyboardF2: return "F2";
		case C64Key::KeyboardF3: return "F3";
		case C64Key::KeyboardF4: return "F4";
		case C64Key::KeyboardF5: return "F5";
		case C64Key::KeyboardF6: return "F6";
		case C64Key::KeyboardF7: return "F7";
		case C64Key::KeyboardF8: return "F8";
		case C64Key::KeyboardLeftArrow: return "←";
		case C64Key::Keyboard1: return "1";
		case C64Key::Keyboard2: return "2";
		case C64Key::Keyboard3: return "3";
		case C64Key::Keyboard4: return "4";
		case C64Key::Keyboard5: return "5";
		case C64Key::Keyboard6: return "6";
		case C64Key::Keyboard7: return "7";
		case C64Key::Keyboard8: return "8";
		case C64Key::Keyboard9: return "9";
		case C64Key::Keyboard0: return "0";
		case C64Key::KeyboardPlus: return "+";
		case C64Key::KeyboardMinus: return "-";
		case C64Key::KeyboardPound: return "£";
		case C64Key::KeyboardClrHome: return "Clr Home";
		case C64Key::KeyboardInstDel: return "Inst Del";
		case C64Key::KeyboardCtrl: return "Ctrl";
		case C64Key::KeyboardQ: return "Q";
		case C64Key::KeyboardW: return "W";
		case C64Key::KeyboardE: return "E";
		case C64Key::KeyboardR: return "R";
		case C64Key::KeyboardT: return "T";
		case C64Key::KeyboardY: return "Y";
		case C64Key::KeyboardU: return "U";
		case C64Key::KeyboardI: return "I";
		case C64Key::KeyboardO: return "O";
		case C64Key::KeyboardP: return "P";
		case C64Key::KeyboardAt: return "@";
		case C64Key::KeyboardAsterisk: return "*";
		case C64Key::KeyboardUpArrow: return "↑";
		case C64Key::KeyboardRestore: return "Restore";
		case C64Key::KeyboardRunStop: return "Run Stop";
		case C64Key::KeyboardShiftLock: return "Shift Lock";
		case C64Key::KeyboardA: return "A";
		case C64Key::KeyboardS: return "S";
		case C64Key::KeyboardD: return "D";
		case C64Key::KeyboardF: return "F";
		case C64Key::KeyboardG: return "G";
		case C64Key::KeyboardH: return "H";
		case C64Key::KeyboardJ: return "J";
		case C64Key::KeyboardK: return "K";
		case C64Key::KeyboardL: return "L";
		case C64Key::KeyboardColon: return ":";
		case C64Key::KeyboardSemiColon: return ";";
		case C64Key::KeyboardEquals: return "=";
		case C64Key::KeyboardReturn: return "Return";
		case C64Key::KeyboardCommodore: return "Commodore Logo";
		case C64Key::KeyboardLeftShift: return "Left Shift";
		case C64Key::KeyboardZ: return "Z";
		case C64Key::KeyboardX: return "X";
		case C64Key::KeyboardC: return "C";
		case C64Key::KeyboardV: return "V";
		case C64Key::KeyboardB: return "B";
		case C64Key::KeyboardN: return "N";
		case C64Key::KeyboardM: return "M";
		case C64Key::KeyboardComma: return ",";
		case C64Key::KeyboardPeriod: return ".";
		case C64Key::KeyboardSlash: return "/";
		case C64Key::KeyboardRightShift: return "Right Shift";
		case C64Key::KeyboardUp: return "Up";
		case C64Key::KeyboardRight: return "Right";
		case C64Key::KeyboardDown: return "Down";
		case C64Key::KeyboardLeft: return "Left";
		case C64Key::KeyboardSpace: return "Space";
		case C64Key::KeyboardCtrlLock: return "Ctrl Lock";
		case C64Key::KeyboardExclam: return "!";
		case C64Key::KeyboardQuoteDbl: return "\"";
		case C64Key::KeyboardNumberSign: return "#";
		case C64Key::KeyboardDollar: return "$";
		case C64Key::KeyboardPercent: return "%";
		case C64Key::KeyboardAmpersand: return "&";
		case C64Key::KeyboardParenLeft: return "(";
		case C64Key::KeyboardParenRight: return ")";
		case C64Key::KeyboardBracketLeft: return "[";
		case C64Key::KeyboardBracketRight: return "]";
		case C64Key::KeyboardLess: return "<";
		case C64Key::KeyboardGreater: return ">";
		case C64Key::KeyboardQuestion: return "?";
		case C64Key::KeyboardApostrophe: return "'";
		default: return "";
	}
}

std::span<const KeyConfigDesc> AppMeta::defaultKeyConfigs()
{
	using namespace Input;

	static constexpr std::array pcKeyboardMap
	{
		KeyMapping{C64Key::Up, Keycode::UP},
		KeyMapping{C64Key::Right, Keycode::RIGHT},
		KeyMapping{C64Key::Down, Keycode::DOWN},
		KeyMapping{C64Key::Left, Keycode::LEFT},
		KeyMapping{C64Key::JSTrigger, Keycode::LALT},
		KeyMapping{C64Key::SwapJSPorts, Keycode::F10},
		KeyMapping{C64Key::KeyboardF1, Keycode::F1},
		KeyMapping{C64Key::KeyboardF2, Keycode::F2},
		KeyMapping{C64Key::KeyboardF3, Keycode::F3},
		KeyMapping{C64Key::KeyboardF4, Keycode::F4},
		KeyMapping{C64Key::KeyboardF5, Keycode::F5},
		KeyMapping{C64Key::KeyboardF6, Keycode::F6},
		KeyMapping{C64Key::KeyboardF7, Keycode::F7},
		KeyMapping{C64Key::KeyboardF8, Keycode::F8},
		KeyMapping{C64Key::Keyboard1, Keycode::_1},
		KeyMapping{C64Key::Keyboard2, Keycode::_2},
		KeyMapping{C64Key::Keyboard3, Keycode::_3},
		KeyMapping{C64Key::Keyboard4, Keycode::_4},
		KeyMapping{C64Key::Keyboard5, Keycode::_5},
		KeyMapping{C64Key::Keyboard6, Keycode::_6},
		KeyMapping{C64Key::Keyboard7, Keycode::_7},
		KeyMapping{C64Key::Keyboard8, Keycode::_8},
		KeyMapping{C64Key::Keyboard9, Keycode::_9},
		KeyMapping{C64Key::Keyboard0, Keycode::_0},
		KeyMapping{C64Key::KeyboardExclam, {Keycode::LSHIFT, Keycode::_1}},
		KeyMapping{C64Key::KeyboardAt, {Keycode::LSHIFT, Keycode::_2}},
		KeyMapping{C64Key::KeyboardNumberSign, {Keycode::LSHIFT, Keycode::_3}},
		KeyMapping{C64Key::KeyboardDollar, {Keycode::LSHIFT, Keycode::_4}},
		KeyMapping{C64Key::KeyboardPercent, {Keycode::LSHIFT, Keycode::_5}},
		KeyMapping{C64Key::KeyboardAmpersand, {Keycode::LSHIFT, Keycode::_7}},
		KeyMapping{C64Key::KeyboardAsterisk, {Keycode::LSHIFT, Keycode::_8}},
		KeyMapping{C64Key::KeyboardParenLeft, {Keycode::LSHIFT, Keycode::_9}},
		KeyMapping{C64Key::KeyboardParenRight, {Keycode::LSHIFT, Keycode::_0}},
		KeyMapping{C64Key::KeyboardMinus, Keycode::MINUS},
		KeyMapping{C64Key::KeyboardLeftArrow, {Keycode::LSHIFT, Keycode::MINUS}},
		KeyMapping{C64Key::KeyboardPound, Keycode::BACKSLASH},
		KeyMapping{C64Key::KeyboardClrHome, Keycode::HOME},
		KeyMapping{C64Key::KeyboardInstDel, Keycode::BACK_SPACE},
		KeyMapping{C64Key::KeyboardCtrl, Keycode::LCTRL},
		KeyMapping{C64Key::KeyboardQ, Keycode::Q},
		KeyMapping{C64Key::KeyboardW, Keycode::W},
		KeyMapping{C64Key::KeyboardE, Keycode::E},
		KeyMapping{C64Key::KeyboardR, Keycode::R},
		KeyMapping{C64Key::KeyboardT, Keycode::T},
		KeyMapping{C64Key::KeyboardY, Keycode::Y},
		KeyMapping{C64Key::KeyboardU, Keycode::U},
		KeyMapping{C64Key::KeyboardI, Keycode::I},
		KeyMapping{C64Key::KeyboardO, Keycode::O},
		KeyMapping{C64Key::KeyboardP, Keycode::P},
		KeyMapping{C64Key::KeyboardUpArrow, Keycode::PGDOWN},
		KeyMapping{C64Key::KeyboardRestore, Keycode::END},
		KeyMapping{C64Key::KeyboardRunStop, Keycode::PAUSE},
		KeyMapping{C64Key::KeyboardShiftLock, Keycode::CAPS},
		KeyMapping{C64Key::KeyboardA, Keycode::A},
		KeyMapping{C64Key::KeyboardS, Keycode::S},
		KeyMapping{C64Key::KeyboardD, Keycode::D},
		KeyMapping{C64Key::KeyboardF, Keycode::F},
		KeyMapping{C64Key::KeyboardG, Keycode::G},
		KeyMapping{C64Key::KeyboardH, Keycode::H},
		KeyMapping{C64Key::KeyboardJ, Keycode::J},
		KeyMapping{C64Key::KeyboardK, Keycode::K},
		KeyMapping{C64Key::KeyboardL, Keycode::L},
		KeyMapping{C64Key::KeyboardSemiColon, Keycode::SEMICOLON},
		KeyMapping{C64Key::KeyboardColon, {Keycode::LSHIFT, Keycode::SEMICOLON}},
		KeyMapping{C64Key::KeyboardEquals, Keycode::EQUALS},
		KeyMapping{C64Key::KeyboardPlus, {Keycode::LSHIFT, Keycode::EQUALS}},
		KeyMapping{C64Key::KeyboardReturn, Keycode::ENTER},
		KeyMapping{C64Key::KeyboardCommodore, Keycode::TAB},
		KeyMapping{C64Key::KeyboardLeftShift, Keycode::LSHIFT},
		KeyMapping{C64Key::KeyboardZ, Keycode::Z},
		KeyMapping{C64Key::KeyboardX, Keycode::X},
		KeyMapping{C64Key::KeyboardC, Keycode::C},
		KeyMapping{C64Key::KeyboardV, Keycode::V},
		KeyMapping{C64Key::KeyboardB, Keycode::B},
		KeyMapping{C64Key::KeyboardN, Keycode::N},
		KeyMapping{C64Key::KeyboardM, Keycode::M},
		KeyMapping{C64Key::KeyboardComma, Keycode::COMMA},
		KeyMapping{C64Key::KeyboardLess, {Keycode::LSHIFT, Keycode::COMMA}},
		KeyMapping{C64Key::KeyboardPeriod, Keycode::PERIOD},
		KeyMapping{C64Key::KeyboardGreater, {Keycode::LSHIFT, Keycode::PERIOD}},
		KeyMapping{C64Key::KeyboardSlash, Keycode::SLASH},
		KeyMapping{C64Key::KeyboardQuestion, {Keycode::LSHIFT, Keycode::SLASH}},
		KeyMapping{C64Key::KeyboardRightShift, Keycode::RSHIFT},
		KeyMapping{C64Key::KeyboardSpace, Keycode::SPACE},
		KeyMapping{C64Key::KeyboardCtrlLock, Keycode::INSERT},
		KeyMapping{C64Key::KeyboardBracketLeft, Keycode::LEFT_BRACKET},
		KeyMapping{C64Key::KeyboardBracketRight, Keycode::RIGHT_BRACKET},
		KeyMapping{C64Key::KeyboardApostrophe, Keycode::APOSTROPHE},
		KeyMapping{C64Key::KeyboardQuoteDbl, {Keycode::LSHIFT, Keycode::APOSTROPHE}},
	};

	static constexpr std::array genericGamepadMap
	{
		KeyMapping{C64Key::Up, Keycode::UP},
		KeyMapping{C64Key::Right, Keycode::RIGHT},
		KeyMapping{C64Key::Down, Keycode::DOWN},
		KeyMapping{C64Key::Left, Keycode::LEFT},
		KeyMapping{C64Key::JSTrigger, Keycode::GAME_A},
		KeyMapping{C64Key::SwapJSPorts, Keycode::GAME_R1},
		KeyMapping{C64Key::ToggleKB, Keycode::GAME_START},
		KeyMapping{C64Key::KeyboardF1, Keycode::GAME_SELECT},
		KeyMapping{C64Key::KeyboardF3, Keycode::GAME_B},
		KeyMapping{C64Key::KeyboardF5, Keycode::GAME_Y},
		KeyMapping{C64Key::KeyboardF7, Keycode::GAME_L1},
	};

	static constexpr std::array wiimoteMap
	{
		KeyMapping{C64Key::Up, WiimoteKey::UP},
		KeyMapping{C64Key::Right, WiimoteKey::RIGHT},
		KeyMapping{C64Key::Down, WiimoteKey::DOWN},
		KeyMapping{C64Key::Left, WiimoteKey::LEFT},
		KeyMapping{C64Key::JSTrigger, WiimoteKey::_1},
		KeyMapping{C64Key::KeyboardF1, WiimoteKey::PLUS},
	};

	return genericKeyConfigs<pcKeyboardMap, genericGamepadMap, wiimoteMap>();
}

bool AppMeta::allowsTurboModifier(KeyCode c)
{
	return C64Key(c) == C64Key::JSTrigger;
}

constexpr FRect gpImageCoords(IRect cellRelBounds)
{
	constexpr F2Size imageSize{256, 128};
	constexpr int cellSize = 32;
	return (cellRelBounds.relToAbs() * cellSize).as<float>() / imageSize;
}

AssetDesc AppMeta::vControllerAssetDesc(KeyInfo key)
{
	static constexpr struct VirtualControllerAssets
	{
		AssetDesc dpad{AssetFileID::gamepadOverlay, gpImageCoords({{}, {4, 4}})},

		jsBtn{AssetFileID::gamepadOverlay, gpImageCoords({{4, 0}, {2, 2}})},

		kb{AssetFileID::gamepadOverlay,          gpImageCoords({{6, 0}, {2, 1}}), {1, 2}},
		swapJsPorts{AssetFileID::gamepadOverlay, gpImageCoords({{6, 1}, {2, 1}}), {1, 2}},

		f1{AssetFileID::gamepadOverlay, gpImageCoords({{4, 2}, {2, 1}}), {1, 2}},
		f3{AssetFileID::gamepadOverlay, gpImageCoords({{6, 2}, {2, 1}}), {1, 2}},
		f5{AssetFileID::gamepadOverlay, gpImageCoords({{4, 3}, {2, 1}}), {1, 2}},
		f7{AssetFileID::gamepadOverlay, gpImageCoords({{6, 3}, {2, 1}}), {1, 2}};
	} virtualControllerAssets;

	if(key[0] == 0)
		return virtualControllerAssets.dpad;
	switch(C64Key(key[0]))
	{
		case C64Key::KeyboardF1: return virtualControllerAssets.f1;
		case C64Key::KeyboardF3: return virtualControllerAssets.f3;
		case C64Key::KeyboardF5: return virtualControllerAssets.f5;
		case C64Key::KeyboardF7: return virtualControllerAssets.f7;
		case C64Key::ToggleKB: return virtualControllerAssets.kb;
		case C64Key::SwapJSPorts: return virtualControllerAssets.swapJsPorts;
		default: return virtualControllerAssets.jsBtn;
	}
}

SystemInputDeviceDesc AppMeta::inputDeviceDesc(int idx)
{
	static constexpr std::array jsComponents
	{
		InputComponentDesc{"D-Pad", dpadKeyInfo, InputComponent::dPad, LB2DO},
		InputComponentDesc{"Joystick Button", triggerKeyInfo, InputComponent::button, RB2DO},
		InputComponentDesc{"F1 & Keyboard Toggle", shortcutKeyInfo, InputComponent::button, RB2DO, {.rowSize = 1}},
	};

	static constexpr SystemInputDeviceDesc jsDesc{"Joystick", jsComponents};

	return jsDesc;
}

void AppMeta::onCustomizeNavView(AppNavView& view)
{
	const Gfx::LGradientStopDesc navViewGrad[] =
	{
		{ .0, Gfx::PackedColor::format.build(48./255., 36./255., 144./255., 1.) },
		{ .3, Gfx::PackedColor::format.build(48./255., 36./255., 144./255., 1.) },
		{ .97, Gfx::PackedColor::format.build((48./255.) * .4, (36./255.) * .4, (144./255.) * .4, 1.) },
		{ 1., view.separatorColor() },
	};
	view.setBackgroundGradient(navViewGrad);
}

}
