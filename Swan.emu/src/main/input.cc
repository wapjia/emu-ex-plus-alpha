/*  This file is part of Swan.emu.

	Swan.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Swan.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Swan.emu.  If not, see <http://www.gnu.org/licenses/> */

module;
#include <mednafen/mednafen.h>

module system;

extern "C++" namespace MDFN_IEN_WSWAN
{
	extern uint16 WSButtonStatus;
}

namespace EmuEx
{

enum KeypadMask: uint16_t
{
	X1_BIT = bit(0),
	X2_BIT = bit(1),
	X3_BIT = bit(2),
	X4_BIT = bit(3),
	Y1_BIT = bit(4),
	Y2_BIT = bit(5),
	Y3_BIT = bit(6),
	Y4_BIT = bit(7),
	START_BIT = bit(8),
	A_BIT = bit(9),
	B_BIT = bit(10),
};

void WsSystem::handleInputAction(EmuApp *, InputAction a)
{
	using namespace MDFN_IEN_WSWAN;
	auto gpBits = [&] -> uint16_t
	{
		if(isRotated())
		{
			switch(SwanKey(a.code))
			{
				case SwanKey::Up: return Y2_BIT;
				case SwanKey::Right: return Y3_BIT;
				case SwanKey::Down: return Y4_BIT;
				case SwanKey::Left: return Y1_BIT;
				case SwanKey::Y1: return B_BIT;
				case SwanKey::Y2: return A_BIT;
				case SwanKey::Y3: return X3_BIT;
				case SwanKey::Y4: return X2_BIT;
				case SwanKey::Start: return START_BIT;
				case SwanKey::A: return X4_BIT;
				case SwanKey::B: return X1_BIT;
				case SwanKey::ANoRotation: return A_BIT;
				case SwanKey::BNoRotation: return B_BIT;
				case SwanKey::Y1X1: return X1_BIT;
				case SwanKey::Y2X2: return X2_BIT;
				case SwanKey::Y3X3: return X3_BIT;
				case SwanKey::Y4X4: return X4_BIT;
			}
		}
		else
		{
			switch(SwanKey(a.code))
			{
				case SwanKey::Up: return X1_BIT;
				case SwanKey::Right: return X2_BIT;
				case SwanKey::Down: return X3_BIT;
				case SwanKey::Left: return X4_BIT;
				case SwanKey::Y1: return Y1_BIT;
				case SwanKey::Y2: return Y2_BIT;
				case SwanKey::Y3: return Y3_BIT;
				case SwanKey::Y4: return Y4_BIT;
				case SwanKey::Start: return START_BIT;
				case SwanKey::A: return A_BIT;
				case SwanKey::B: return B_BIT;
				case SwanKey::ANoRotation: return A_BIT;
				case SwanKey::BNoRotation: return B_BIT;
				case SwanKey::Y1X1: return Y1_BIT;
				case SwanKey::Y2X2: return Y2_BIT;
				case SwanKey::Y3X3: return Y3_BIT;
				case SwanKey::Y4X4: return Y4_BIT;
			}
		}
		unreachable();
	}();
	WSButtonStatus = setOrClearBits(WSButtonStatus, gpBits, a.isPushed());
}

void WsSystem::clearInputBuffers()
{
	MDFN_IEN_WSWAN::WSButtonStatus = {};
}

void WsSystem::setupInput(EmuApp &app)
{
	constexpr std::array faceKeyInfo{KeyCode(SwanKey::BNoRotation), KeyCode(SwanKey::ANoRotation)};
	constexpr std::array oppositeDPadKeyInfo{KeyCode(SwanKey::Y4X4), KeyCode(SwanKey::Y3X3),
		KeyCode(SwanKey::Y1X1), KeyCode(SwanKey::Y2X2)};
	if(isRotated())
	{
		if(showVGamepadABWhenVertical)
			app.unsetDisabledInputKeys();
		else
			app.setDisabledInputKeys(faceKeyInfo);
	}
	else
	{
		if(showVGamepadYWhenHorizonal)
			app.unsetDisabledInputKeys();
		else
			app.setDisabledInputKeys(oppositeDPadKeyInfo);
	}
}

}
