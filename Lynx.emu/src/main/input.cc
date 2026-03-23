/*  This file is part of Lynx.emu.

	Lynx.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Lynx.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Lynx.emu.  If not, see <http://www.gnu.org/licenses/> */

module;
#include <mednafen/mednafen.h>

module system;

extern "C++" void Lynx_SetButtonData(uint32 data);

namespace EmuEx
{

static LynxKey rotateDPadKeycode(LynxKey key, Rotation rotation)
{
	if(rotation == Rotation::UP)
		return key;
	switch(key)
	{
		case LynxKey::Up: return rotation == Rotation::RIGHT ? LynxKey::Right : LynxKey::Left;
		case LynxKey::Right: return rotation == Rotation::RIGHT ? LynxKey::Down : LynxKey::Up;
		case LynxKey::Down: return rotation == Rotation::RIGHT ? LynxKey::Left : LynxKey::Right;
		case LynxKey::Left: return rotation == Rotation::RIGHT ? LynxKey::Up : LynxKey::Down;
		default: return key;
	}
}

LynxKey LynxSystem::rotateDPadKeycode(LynxKey key) const
{
	return EmuEx::rotateDPadKeycode(key, contentRotation());
}

void LynxSystem::handleInputAction(EmuApp *, InputAction a)
{
	auto key = rotateDPadKeycode(LynxKey(a.code));
	inputBuff = setOrClearBits(inputBuff, bit(to_underlying(key) - 1), a.isPushed());
	Lynx_SetButtonData(inputBuff);
}

void LynxSystem::clearInputBuffers()
{
	inputBuff = {};
	Lynx_SetButtonData(0);
}

}
