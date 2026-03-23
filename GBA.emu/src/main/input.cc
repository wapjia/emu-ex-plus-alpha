/*  This file is part of GBA.emu.

	GBA.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	GBA.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with GBA.emu.  If not, see <http://www.gnu.org/licenses/> */

module;
#include <core/gba/gba.h>
#include <core/gba/gbaGlobals.h>

module system;

namespace EmuEx
{

using namespace IG;

void GbaSystem::handleInputAction(EmuApp *app, InputAction a)
{
	auto key = GbaKey(a.code);
	switch(key)
	{
		case GbaKey::LightInc:
		case GbaKey::LightDec:
		{
			int darknessChange = key == GbaKey::LightDec ? 17 : -17;
			darknessLevel = std::clamp(darknessLevel + darknessChange, 0, 0xff);
			if(app)
			{
				app->postMessage(1, false, std::format("Light sensor level: {}%", remap(darknessLevel, 0xff, 0, 0, 100)));
			}
			break;
		}
		default:
			auto& P1 = gGba.mem.ioMem.P1;
			P1 = setOrClearBits(P1, bit(a.code - 1), !a.isPushed());
			break;
	}
}

void GbaSystem::clearInputBuffers()
{
	gGba.mem.ioMem.P1 = 0x03FF;
	clearSensorValues();
}

}
