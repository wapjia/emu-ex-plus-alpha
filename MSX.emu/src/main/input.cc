/*  This file is part of MSX.emu.

	MSX.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	MSX.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with MSX.emu.  If not, see <http://www.gnu.org/licenses/> */

module;
extern "C"
{
	#include <blueMSX/Input/InputEvent.h>
	#include <blueMSX/Board/Board.h>
}

module system;

namespace EmuEx
{

static VController::KbMap kbToEventMap
{
	EC_Q, EC_W, EC_E, EC_R, EC_T, EC_Y, EC_U, EC_I, EC_O, EC_P,
	EC_A, EC_S, EC_D, EC_F, EC_G, EC_H, EC_J, EC_K, EC_L, EC_NONE,
	EC_CAPS, EC_Z, EC_X, EC_C, EC_V, EC_B, EC_N, EC_M, EC_BKSPACE, EC_NONE,
	EC_NONE, EC_NONE, EC_NONE, EC_SPACE, EC_SPACE, EC_SPACE, EC_SPACE, EC_CTRL, EC_CTRL, EC_RETURN
};

static VController::KbMap kbToEventMap2
{
	EC_F1, EC_F1, EC_F2, EC_F2, EC_F3, EC_F3, EC_F4, EC_F4, EC_F5, EC_F5, // 0-9
	EC_1, EC_2, EC_3, EC_4, EC_5, EC_6, EC_7, EC_8, EC_9, EC_0, // 10-19
	EC_TAB, std::array{EC_8, EC_LSHIFT}, std::array{EC_9, EC_LSHIFT}, std::array{EC_3, EC_LSHIFT}, std::array{EC_4, EC_LSHIFT}, std::array{EC_SEMICOL, EC_LSHIFT}, EC_NEG, EC_SEMICOL, EC_ESC, EC_NONE,
	EC_NONE, EC_NONE, EC_NONE, EC_SPACE, EC_SPACE, EC_SPACE, EC_SPACE, EC_PERIOD, EC_PERIOD, EC_RETURN
};

void setupVKeyboardMap(EmuApp &app, unsigned boardType)
{
	if(boardType != BOARD_COLECO)
	{
		for(auto i: iotaCount(10)) // 1 - 0
			kbToEventMap2[10 + i] = EC_1 + i;
		kbToEventMap2[23] = std::array{EC_3, EC_LSHIFT};
	}
	else
	{
		for(auto i: iotaCount(9)) // 1 - 9
			kbToEventMap2[10 + i] = EC_COLECO1_1 + i;
		kbToEventMap2[19] = EC_COLECO1_0;
		kbToEventMap2[23] = EC_COLECO1_HASH;
	}
	app.inputManager.updateKeyboardMapping();
}

VController::KbMap MsxSystem::vControllerKeyboardMap(VControllerKbMode mode)
{
	return mode == VControllerKbMode::LAYOUT_2 ? kbToEventMap2 : kbToEventMap;
}

void MsxSystem::handleInputAction(EmuApp *appPtr, InputAction a)
{
	if(a.code == EC_KEYCOUNT)
	{
		if(appPtr && a.isPushed())
			appPtr->inputManager.toggleKeyboard();
	}
	else
	{
		assume(a.code < EC_KEYCOUNT);
		auto keyIdx = [&] -> int
		{
			bool isPort1 = a.flags.deviceId == 0;
			switch(a.code)
			{
				case EC_JOY1_UP ... EC_JOY1_BUTTON2:
					return isPort1 ? a.code : a.code + 10;
				case EC_COLECO1_0 ... EC_COLECO1_HASH:
					return isPort1 ? a.code : a.code + 20;
				default:
					return activeBoardType == BOARD_COLECO ? EC_COLECO1_STAR : a.code;
			}
		}();
		eventMap[keyIdx] = a.isPushed();
	}
}

void MsxSystem::clearInputBuffers()
{
	fill(eventMap);
}

}
