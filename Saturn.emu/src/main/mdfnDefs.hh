#pragma once

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

#include <mednafen/mednafen.h>

namespace Mednafen
{
class CDInterface;
}

namespace MDFN_IEN_SS
{
extern Mednafen::CDInterface* Cur_CDIF;
extern const int ActiveCartType;
extern uint8 AreaCode;
}

namespace MDFN_IEN_SS::VDP2
{
extern const uint8 InterlaceMode;
extern const bool PAL;
}
