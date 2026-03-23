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

module;
#include <cstdlib>
extern "C"
{
	#include "vice.h"
	#include "kbd.h"
	#include "keyboard.h"
	#include "joyport.h"
}

module system;

namespace EmuEx
{

VController::KbMap C64System::vControllerKeyboardMap(VControllerKbMode mode)
{
	static constexpr VController::KbMap kbToEventMap =
	{
		KeyCode(C64Key::KeyboardQ), KeyCode(C64Key::KeyboardW), KeyCode(C64Key::KeyboardE), KeyCode(C64Key::KeyboardR), KeyCode(C64Key::KeyboardT), KeyCode(C64Key::KeyboardY), KeyCode(C64Key::KeyboardU), KeyCode(C64Key::KeyboardI), KeyCode(C64Key::KeyboardO), KeyCode(C64Key::KeyboardP),
		KeyCode(C64Key::KeyboardA), KeyCode(C64Key::KeyboardS), KeyCode(C64Key::KeyboardD), KeyCode(C64Key::KeyboardF), KeyCode(C64Key::KeyboardG), KeyCode(C64Key::KeyboardH), KeyCode(C64Key::KeyboardJ), KeyCode(C64Key::KeyboardK), KeyCode(C64Key::KeyboardL), 0,
		KeyCode(C64Key::KeyboardShiftLock), KeyCode(C64Key::KeyboardZ), KeyCode(C64Key::KeyboardX), KeyCode(C64Key::KeyboardC), KeyCode(C64Key::KeyboardV), KeyCode(C64Key::KeyboardB), KeyCode(C64Key::KeyboardN), KeyCode(C64Key::KeyboardM), KeyCode(C64Key::KeyboardInstDel), 0,
		0, 0, 0, KeyCode(C64Key::KeyboardSpace), KeyCode(C64Key::KeyboardSpace), KeyCode(C64Key::KeyboardSpace), KeyCode(C64Key::KeyboardSpace), KeyCode(C64Key::KeyboardRunStop), KeyCode(C64Key::KeyboardRunStop), KeyCode(C64Key::KeyboardReturn)
	};

	static constexpr VController::KbMap kbToEventMap2 =
	{
		KeyCode(C64Key::KeyboardF1), KeyCode(C64Key::KeyboardF3), KeyCode(C64Key::KeyboardF5), KeyCode(C64Key::KeyboardF7), KeyCode(C64Key::KeyboardAt), KeyCode(C64Key::KeyboardCommodore), KeyCode(C64Key::KeyboardLeftArrow), KeyCode(C64Key::KeyboardUpArrow), KeyCode(C64Key::KeyboardPlus), KeyCode(C64Key::KeyboardMinus),
		KeyCode(C64Key::Keyboard1), KeyCode(C64Key::Keyboard2), KeyCode(C64Key::Keyboard3), KeyCode(C64Key::Keyboard4), KeyCode(C64Key::Keyboard5), KeyCode(C64Key::Keyboard6), KeyCode(C64Key::Keyboard7), KeyCode(C64Key::Keyboard8), KeyCode(C64Key::Keyboard9), KeyCode(C64Key::Keyboard0),
		KeyCode(C64Key::KeyboardRestore), KeyCode(C64Key::KeyboardColon), KeyCode(C64Key::KeyboardSemiColon), KeyCode(C64Key::KeyboardEquals), KeyCode(C64Key::KeyboardComma), KeyCode(C64Key::KeyboardPeriod), KeyCode(C64Key::KeyboardSlash), KeyCode(C64Key::KeyboardAsterisk), KeyCode(C64Key::KeyboardClrHome), 0,
		0, 0, 0, KeyCode(C64Key::KeyboardSpace), KeyCode(C64Key::KeyboardSpace), KeyCode(C64Key::KeyboardSpace), KeyCode(C64Key::KeyboardSpace), KeyCode(C64Key::KeyboardCtrlLock), KeyCode(C64Key::KeyboardCtrlLock), KeyCode(C64Key::KeyboardReturn)
	};

	return mode == VControllerKbMode::LAYOUT_2 ? kbToEventMap2 : kbToEventMap;
}

static KeyCode shiftKeycodePositional(C64Key keycode)
{
	switch(keycode)
	{
		case C64Key::KeyboardColon: return KeyCode(C64Key::KeyboardBracketLeft);
		case C64Key::KeyboardSemiColon: return KeyCode(C64Key::KeyboardBracketRight);
		case C64Key::KeyboardComma: return KeyCode(C64Key::KeyboardLess);
		case C64Key::KeyboardPeriod: return KeyCode(C64Key::KeyboardGreater);
		case C64Key::KeyboardSlash: return KeyCode(C64Key::KeyboardQuestion);
		case C64Key::Keyboard1: return KeyCode(C64Key::KeyboardExclam);
		case C64Key::Keyboard2: return KeyCode(C64Key::KeyboardQuoteDbl);
		case C64Key::Keyboard3: return KeyCode(C64Key::KeyboardNumberSign);
		case C64Key::Keyboard4: return KeyCode(C64Key::KeyboardDollar);
		case C64Key::Keyboard5: return KeyCode(C64Key::KeyboardPercent);
		case C64Key::Keyboard6: return KeyCode(C64Key::KeyboardAmpersand);
		case C64Key::Keyboard7: return KeyCode(C64Key::KeyboardApostrophe);
		case C64Key::Keyboard8: return KeyCode(C64Key::KeyboardParenLeft);
		case C64Key::Keyboard9: return KeyCode(C64Key::KeyboardParenRight);
		default: return KeyCode(keycode);
	}
}

void C64System::handleKeyboardInput(InputAction a, bool positionalShift)
{
	int mod{};
	if(a.metaState & Input::Meta::SHIFT)
	{
		mod |= KBD_MOD_LSHIFT;
		if(positionalShift)
			a.code = shiftKeycodePositional(C64Key(a.code));
	}
	if(a.metaState & Input::Meta::CAPS_LOCK)
	{
		mod |= KBD_MOD_SHIFTLOCK;
	}
	plugin.keyboard_key_pressed_direct(a.code, mod, a.isPushed());
}

void C64System::handleInputAction(EmuApp* app, InputAction a)
{
	bool positionalShift{};
	if(app)
	{
		if(app->defaultVController().keyboard().shiftIsActive())
		{
			a.metaState |= Input::Meta::SHIFT;
			positionalShift = true;
		}
	}
	auto key = C64Key(a.code);
	switch(key)
	{
		case C64Key::Up ... C64Key::JSTrigger:
		{
			if(effectiveJoystickMode == JoystickMode::Keyboard)
			{
				if(key == C64Key::Right)
					handleKeyboardInput({KeyCode(C64Key::KeyboardRight), {}, a.state, a.metaState}, positionalShift);
				else if(key == C64Key::Left)
					handleKeyboardInput({KeyCode(C64Key::KeyboardLeft), {}, a.state, a.metaState}, positionalShift);
				else if(key == C64Key::Up)
					handleKeyboardInput({KeyCode(C64Key::KeyboardUp), {}, a.state, a.metaState}, positionalShift);
				else if(key == C64Key::Down)
					handleKeyboardInput({KeyCode(C64Key::KeyboardDown), {}, a.state, a.metaState}, positionalShift);
				else if(key == C64Key::JSTrigger)
					handleKeyboardInput({KeyCode(C64Key::KeyboardPound), {}, a.state, a.metaState}, positionalShift);
			}
			else
			{
				auto &joystick_value = *plugin.joystick_value;
				auto player = a.flags.deviceId;
				if(effectiveJoystickMode == JoystickMode::Port2)
				{
					player = (player == 1) ? 0 : 1;
				}
				auto jsBits = [&] -> uint16_t
				{
					static constexpr uint16_t JS_FIRE = 0x10,
						JS_E = 0x08,
						JS_W = 0x04,
						JS_S = 0x02,
						JS_N = 0x01;
					switch(key)
					{
						case C64Key::Up: return JS_N;
						case C64Key::Right: return JS_E;
						case C64Key::Down: return JS_S;
						case C64Key::Left: return JS_W;
						case C64Key::JSTrigger: return JS_FIRE;
						default: unreachable();
					}
				}();
				joystick_value[player] = setOrClearBits(joystick_value[player], jsBits, a.isPushed());
			}
			break;
		}
		case C64Key::SwapJSPorts:
		{
			if(a.isPushed() && effectiveJoystickMode != JoystickMode::Keyboard)
			{
				EmuSystem::sessionOptionSet();
				if(effectiveJoystickMode == JoystickMode::Port2)
					joystickMode = JoystickMode::Port1;
				else
					joystickMode = JoystickMode::Port2;
				effectiveJoystickMode = joystickMode;
				fill(*plugin.joystick_value);
				if(app)
					app->postMessage(1, false, "Swapped Joystick Ports");
			}
			break;
		}
		case C64Key::ToggleKB:
		{
			if(app && a.state == Input::Action::PUSHED)
				app->inputManager.toggleKeyboard();
			break;
		}
		case C64Key::KeyboardRestore:
		{
			if(app)
			{
				log.info("pushed restore key");
				auto emuThreadResumer = app->suspendEmulationThread();
				plugin.machine_set_restore_key(a.state == Input::Action::PUSHED);
			}
			break;
		}
		case C64Key::KeyboardCtrlLock:
		{
			if(a.isPushed())
			{
				ctrlLock ^= true;
				handleKeyboardInput({KeyCode(C64Key::KeyboardCtrl), {}, ctrlLock ? Input::Action::PUSHED : Input::Action::RELEASED});
			}
			break;
		}
		case C64Key::KeyboardShiftLock:
		{
			if(app && a.isPushed())
			{
				bool active = app->defaultVController().keyboard().toggleShiftActive();
				//log.debug("positional shift:{}", active);
				handleKeyboardInput({KeyCode(C64Key::KeyboardLeftShift), {}, active ? Input::Action::PUSHED : Input::Action::RELEASED});
			}
			break;
		}
		default:
		{
			handleKeyboardInput({a.code, {}, a.state, a.metaState}, positionalShift);
			break;
		}
	}
}

void C64System::clearInputBuffers()
{
	ctrlLock = false;
	auto &joystick_value = *plugin.joystick_value;
	fill(joystick_value);
	plugin.keyboard_key_clear();
}

void C64System::onVKeyboardShown(VControllerKeyboard& kb, bool shown)
{
	if(!shown)
	{
		if(ctrlLock)
		{
			ctrlLock = false;
			handleKeyboardInput({KeyCode(C64Key::KeyboardCtrl), {}, Input::Action::RELEASED});
		}
		if(kb.shiftIsActive())
		{
			kb.setShiftActive(false);
			handleKeyboardInput({KeyCode(C64Key::KeyboardLeftShift), {}, Input::Action::RELEASED});
		}
	}
}

void C64System::setJoystickMode(JoystickMode mode)
{
	joystickMode = mode;
	effectiveJoystickMode = mode == JoystickMode::Auto ? defaultJoystickMode : mode;
	updateJoystickDevices();
}

void C64System::updateJoystickDevices()
{
	enterCPUTrap();
	if(effectiveJoystickMode == JoystickMode::Keyboard)
	{
		setIntResource("JoyPort1Device", JOYPORT_ID_NONE);
		setIntResource("JoyPort2Device", JOYPORT_ID_NONE);
	}
	else
	{
		setIntResource("JoyPort1Device", JOYPORT_ID_JOYSTICK);
		setIntResource("JoyPort2Device", JOYPORT_ID_JOYSTICK);
	}
}

}

extern "C" signed long kbd_arch_keyname_to_keynum(char* keynamePtr)
{
	using namespace EmuEx;
	//log.debug("kbd_arch_keyname_to_keynum({})", keyname);
	std::string_view keyname{keynamePtr};
	if(keyname == "F1") { return long(C64Key::KeyboardF1); }
	else if(keyname == "F2") { return long(C64Key::KeyboardF2); }
	else if(keyname == "F3") { return long(C64Key::KeyboardF3); }
	else if(keyname == "F4") { return long(C64Key::KeyboardF4); }
	else if(keyname == "F5") { return long(C64Key::KeyboardF5); }
	else if(keyname == "F6") { return long(C64Key::KeyboardF6); }
	else if(keyname == "F7") { return long(C64Key::KeyboardF7); }
	else if(keyname == "F8") { return long(C64Key::KeyboardF8); }
	else if(keyname == "End") { return long(C64Key::KeyboardLeftArrow); }
	else if(keyname == "1") { return long(C64Key::Keyboard1); }
	else if(keyname == "2") { return long(C64Key::Keyboard2); }
	else if(keyname == "3") { return long(C64Key::Keyboard3); }
	else if(keyname == "4") { return long(C64Key::Keyboard4); }
	else if(keyname == "5") { return long(C64Key::Keyboard5); }
	else if(keyname == "6") { return long(C64Key::Keyboard6); }
	else if(keyname == "7") { return long(C64Key::Keyboard7); }
	else if(keyname == "8") { return long(C64Key::Keyboard8); }
	else if(keyname == "9") { return long(C64Key::Keyboard9); }
	else if(keyname == "0") { return long(C64Key::Keyboard0); }
	else if(keyname == "plus") { return long(C64Key::KeyboardPlus); }
	else if(keyname == "minus") { return long(C64Key::KeyboardMinus); }
	else if(keyname == "sterling") { return long(C64Key::KeyboardPound); }
	else if(keyname == "Home") { return long(C64Key::KeyboardClrHome); }
	else if(keyname == "BackSpace") { return long(C64Key::KeyboardInstDel); }
	else if(keyname == "Control_L") { return long(C64Key::KeyboardCtrl); }
	else if(keyname == "q") { return long(C64Key::KeyboardQ); }
	else if(keyname == "w") { return long(C64Key::KeyboardW); }
	else if(keyname == "e") { return long(C64Key::KeyboardE); }
	else if(keyname == "r") { return long(C64Key::KeyboardR); }
	else if(keyname == "t") { return long(C64Key::KeyboardT); }
	else if(keyname == "y") { return long(C64Key::KeyboardY); }
	else if(keyname == "u") { return long(C64Key::KeyboardU); }
	else if(keyname == "i") { return long(C64Key::KeyboardI); }
	else if(keyname == "o") { return long(C64Key::KeyboardO); }
	else if(keyname == "p") { return long(C64Key::KeyboardP); }
	else if(keyname == "at") { return long(C64Key::KeyboardAt); }
	else if(keyname == "asterisk") { return long(C64Key::KeyboardAsterisk); }
	else if(keyname == "Page_Down") { return long(C64Key::KeyboardUpArrow); }
	else if(keyname == "Escape") { return long(C64Key::KeyboardRunStop); }
	else if(keyname == "a") { return long(C64Key::KeyboardA); }
	else if(keyname == "s") { return long(C64Key::KeyboardS); }
	else if(keyname == "d") { return long(C64Key::KeyboardD); }
	else if(keyname == "f") { return long(C64Key::KeyboardF); }
	else if(keyname == "g") { return long(C64Key::KeyboardG); }
	else if(keyname == "h") { return long(C64Key::KeyboardH); }
	else if(keyname == "j") { return long(C64Key::KeyboardJ); }
	else if(keyname == "k") { return long(C64Key::KeyboardK); }
	else if(keyname == "l") { return long(C64Key::KeyboardL); }
	else if(keyname == "colon") { return long(C64Key::KeyboardColon); }
	else if(keyname == "semicolon") { return long(C64Key::KeyboardSemiColon); }
	else if(keyname == "equal") { return long(C64Key::KeyboardEquals); }
	else if(keyname == "Return") { return long(C64Key::KeyboardReturn); }
	else if(keyname == "Tab") { return long(C64Key::KeyboardCommodore); }
	else if(keyname == "Shift_L") { return long(C64Key::KeyboardLeftShift); }
	else if(keyname == "z") { return long(C64Key::KeyboardZ); }
	else if(keyname == "x") { return long(C64Key::KeyboardX); }
	else if(keyname == "c") { return long(C64Key::KeyboardC); }
	else if(keyname == "v") { return long(C64Key::KeyboardV); }
	else if(keyname == "b") { return long(C64Key::KeyboardB); }
	else if(keyname == "n") { return long(C64Key::KeyboardN); }
	else if(keyname == "m") { return long(C64Key::KeyboardM); }
	else if(keyname == "comma") { return long(C64Key::KeyboardComma); }
	else if(keyname == "period") { return long(C64Key::KeyboardPeriod); }
	else if(keyname == "slash") { return long(C64Key::KeyboardSlash); }
	else if(keyname == "Shift_R") { return long(C64Key::KeyboardRightShift); }
	else if(keyname == "Up") { return long(C64Key::KeyboardUp); }
	else if(keyname == "Right") { return long(C64Key::KeyboardRight); }
	else if(keyname == "Down") { return long(C64Key::KeyboardDown); }
	else if(keyname == "Left") { return long(C64Key::KeyboardLeft); }
	else if(keyname == "space") { return long(C64Key::KeyboardSpace); }
	else if(keyname == "exclam") { return long(C64Key::KeyboardExclam); }
	else if(keyname == "quotedbl") { return long(C64Key::KeyboardQuoteDbl); }
	else if(keyname == "numbersign") { return long(C64Key::KeyboardNumberSign); }
	else if(keyname == "dollar") { return long(C64Key::KeyboardDollar); }
	else if(keyname == "percent") { return long(C64Key::KeyboardPercent); }
	else if(keyname == "ampersand") { return long(C64Key::KeyboardAmpersand); }
	else if(keyname == "parenleft") { return long(C64Key::KeyboardParenLeft); }
	else if(keyname == "parenright") { return long(C64Key::KeyboardParenRight); }
	else if(keyname == "bracketleft") { return long(C64Key::KeyboardBracketLeft); }
	else if(keyname == "bracketright") { return long(C64Key::KeyboardBracketRight); }
	else if(keyname == "less") { return long(C64Key::KeyboardLess); }
	else if(keyname == "greater") { return long(C64Key::KeyboardGreater); }
	else if(keyname == "question") { return long(C64Key::KeyboardQuestion); }
	else if(keyname == "apostrophe") { return long(C64Key::KeyboardApostrophe); }
	else if(keyname == "Caps_Lock") { return long(C64Key::KeyboardShiftLock); }
	//logWarn("unknown keyname:%s", keyname.data());
	return 0;
}
