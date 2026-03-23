/*  This file is part of 2600.emu.

	2600.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	2600.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with 2600.emu.  If not, see <http://www.gnu.org/licenses/> */

module;
#include <EventHandler.hxx>
#include <Console.hxx>

module system;

namespace EmuEx
{

void A2600System::clearInputBuffers()
{
	Event &ev = osystem.eventHandler().event();
	ev.clear();

	ev.set(Event::ConsoleLeftDiffB, p1DiffB);
	ev.set(Event::ConsoleLeftDiffA, !p1DiffB);
	ev.set(Event::ConsoleRightDiffB, p2DiffB);
	ev.set(Event::ConsoleRightDiffA, !p2DiffB);
	ev.set(Event::ConsoleColor, vcsColor);
	ev.set(Event::ConsoleBlackWhite, !vcsColor);
}

void A2600System::updateJoytickMapping(EmuApp& app, Controller::Type type)
{
	if(type == Controller::Type::Paddles)
	{
		jsFireMap = {Event::LeftPaddleAFire, Event::LeftPaddleBFire};
		jsLeftMap = {Event::LeftPaddleAIncrease, Event::LeftPaddleBIncrease};
		jsRightMap = {Event::LeftPaddleADecrease, Event::LeftPaddleBDecrease};
	}
	else
	{
		jsFireMap = {Event::LeftJoystickFire, Event::RightJoystickFire};
		jsLeftMap = {Event::LeftJoystickLeft, Event::RightJoystickLeft};
		jsRightMap = {Event::LeftJoystickRight, Event::RightJoystickRight};
	}
}

void A2600System::handleInputAction(EmuApp* app, InputAction act)
{
	auto &ev = osystem.eventHandler().event();
	switch(act.code)
	{
		case Event::ConsoleLeftDiffToggle:
			if(!act.isPushed())
				break;
			p1DiffB ^= true;
			if(app)
			{
				app->postMessage(1, false, p1DiffB ? "P1 Difficulty -> B" : "P1 Difficulty -> A");
			}
			ev.set(Event::ConsoleLeftDiffB, p1DiffB);
			ev.set(Event::ConsoleLeftDiffA, !p1DiffB);
			break;
		case Event::ConsoleRightDiffToggle:
			if(!act.isPushed())
				break;
			p2DiffB ^= true;
			if(app)
			{
				app->postMessage(1, false, p2DiffB ? "P2 Difficulty -> B" : "P2 Difficulty -> A");
			}
			ev.set(Event::ConsoleRightDiffB, p2DiffB);
			ev.set(Event::ConsoleRightDiffA, !p2DiffB);
			break;
		case Event::ConsoleColorToggle:
			if(!act.isPushed())
				break;
			vcsColor ^= true;
			if(app)
			{
				app->postMessage(1, false, vcsColor ? "Color Switch -> Color" : "Color Switch -> B&W");
			}
			ev.set(Event::ConsoleColor, vcsColor);
			ev.set(Event::ConsoleBlackWhite, !vcsColor);
			break;
		default:
		{
			auto e = [&] -> Event::Type
			{
				bool isLeftPort = act.flags.deviceId == 0;
				if(isLeftPort)
				{
					switch(act.code)
					{
						case Event::LeftJoystickRight: return jsRightMap[0];
						case Event::LeftJoystickLeft: return jsLeftMap[0];
						case Event::LeftJoystickFire: return jsFireMap[0];
					}
				}
				else
				{
					switch(act.code)
					{
						case Event::LeftJoystickUp: return Event::RightJoystickUp;
						case Event::LeftJoystickRight: return jsRightMap[1];
						case Event::LeftJoystickDown: return Event::RightJoystickDown;
						case Event::LeftJoystickLeft: return jsLeftMap[1];
						case Event::LeftJoystickFire: return jsFireMap[1];
						case Event::LeftKeyboard1 ... Event::LeftKeyboardPound:
							return Event::Type(act.code + (Event::RightKeyboard1 - Event::LeftKeyboard1));
					}
				}
				return Event::Type(act.code);
			}();
			ev.set(e, act.isPushed());
			break;
		}
	}
}

static void updateVirtualDPad(EmuApp& app, Console& console, PaddleRegionMode mode)
{
	auto leftController = console.leftController().type();
	if(leftController == Controller::Type::Paddles)
	{
		app.defaultVController().setGamepadDPadIsEnabled(mode == PaddleRegionMode::OFF);
	}
	else
	{
		app.defaultVController().setGamepadDPadIsEnabled(leftController != Controller::Type::Keyboard);
	}
}

void A2600System::updatePaddlesRegionMode(EmuApp& app, PaddleRegionMode mode)
{
	optionPaddleAnalogRegion = (uint8_t)mode;
	updateVirtualDPad(app, osystem.console(), mode);
}

void A2600System::setControllerType(EmuApp& app, Console& console, Controller::Type type)
{
	static constexpr std::array js1ButtonCodes{KeyCode(Event::LeftJoystickFire)};
	static constexpr std::array js2ButtonCodes{KeyCode(Event::LeftJoystickFire5)};
	static constexpr std::array js3ButtonCodes{KeyCode(Event::LeftJoystickFire9)};
	static constexpr std::array kbButtonCodes
	{
		KeyCode(Event::LeftKeyboard1),
		KeyCode(Event::LeftKeyboard2),
		KeyCode(Event::LeftKeyboard3),
		KeyCode(Event::LeftKeyboard4),
		KeyCode(Event::LeftKeyboard5),
		KeyCode(Event::LeftKeyboard6),
		KeyCode(Event::LeftKeyboard7),
		KeyCode(Event::LeftKeyboard8),
		KeyCode(Event::LeftKeyboard9),
		KeyCode(Event::LeftKeyboardStar),
		KeyCode(Event::LeftKeyboard0),
		KeyCode(Event::LeftKeyboardPound),
	};
	if(type == Controller::Type::Unknown)
		type = autoDetectedInput1;
	if(type == Controller::Type::Genesis)
	{
		app.setDisabledInputKeys(concatToArrayNow<kbButtonCodes, js3ButtonCodes>);
	}
	else if(type == Controller::Type::BoosterGrip)
	{
		app.setDisabledInputKeys(kbButtonCodes);
	}
	else if(type == Controller::Type::Keyboard)
	{
		app.setDisabledInputKeys(concatToArrayNow<js1ButtonCodes, js2ButtonCodes, js3ButtonCodes>);
	}
	else // joystick
	{
		app.setDisabledInputKeys(concatToArrayNow<kbButtonCodes, js2ButtonCodes, js3ButtonCodes>);
	}
	updateVirtualDPad(app, console, (PaddleRegionMode)optionPaddleAnalogRegion.value());
	updateJoytickMapping(app, type);
	Controller &currentController = console.leftController();
	if(currentController.type() == type)
	{
		log.info("using controller type:{}", asString(type));
		return;
	}
	auto props = console.properties();
	props.set(PropType::Controller_Left, Controller::getPropName(type));
	props.set(PropType::Controller_Right, Controller::getPropName(type));
	const string& md5 = props.get(PropType::Cart_MD5);
	console.setProperties(props);
	console.setControllers(md5);
	if(Config::DEBUG_BUILD)
	{
		log.info("current controller name in console object:%s", console.leftController().name());
	}
	log.info("set controller to type:{}", asString(type));
}

bool A2600System::updatePaddle(Input::DragTrackerState dragState)
{
	auto regionMode = (PaddleRegionMode)optionPaddleAnalogRegion.value();
	if(regionMode == PaddleRegionMode::OFF)
		return false;
	auto &app = osystem.app();
	int regionXStart = 0;
	int regionXEnd = app.viewController().inputView.viewRect().size().x;
	if(regionMode == PaddleRegionMode::LEFT)
	{
		regionXEnd /= 2;
	}
	else if(regionMode == PaddleRegionMode::RIGHT)
	{
		regionXStart = regionXEnd / 2;
	}
	auto pos = remap(dragState.pos().x, regionXStart, regionXEnd, -32768 / 2, 32767 / 2);
	pos = std::clamp(pos, -32768, 32767);
	auto evType = app.defaultVController().inputPlayer() == 0 ? Event::LeftPaddleAAnalog : Event::LeftPaddleBAnalog;
	osystem.eventHandler().event().set(evType, pos);
	//log.debug("set paddle position:{}", pos);
	return true;
}

bool A2600System::onPointerInputStart(const Input::MotionEvent&, Input::DragTrackerState dragState, WindowRect)
{
	switch(osystem.console().leftController().type())
	{
		case Controller::Type::Paddles:
		{
			return updatePaddle(dragState);
		}
		default:
			return false;
	}
}

bool A2600System::onPointerInputUpdate(const Input::MotionEvent&, Input::DragTrackerState dragState,
	Input::DragTrackerState, WindowRect)
{
	switch(osystem.console().leftController().type())
	{
		case Controller::Type::Paddles:
		{
			return updatePaddle(dragState);
		}
		default:
			return false;
	}
}

}
