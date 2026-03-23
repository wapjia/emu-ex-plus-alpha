#pragma once

/*  This file is part of PCE.emu.

	PCE.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	PCE.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with PCE.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <mednafen/mednafen.h>
#include <pce/pce.h>
#include <pce/vce.h>
#undef DECLFR
#undef DECLFW
#include <pce_fast/pce.h>
#include <pce_fast/vdc.h>

extern const Mednafen::MDFNGI EmulatedPCE_Fast;
extern const Mednafen::MDFNGI EmulatedPCE;

namespace Mednafen
{
	class CDInterface;

	void SCSICD_SetDisc(bool tray_open, CDInterface* cdif, bool no_emu_side_effects = false);
}

namespace MDFN_IEN_PCE_FAST
{
	extern vce_t vce;

	void SetSoundRate(double rate);
	double GetSoundRate();
	void PCECD_Drive_SetDisc(bool tray_open, Mednafen::CDInterface* cdif, bool no_emu_side_effects = false) MDFN_COLD;
}

namespace MDFN_IEN_PCE
{
	using namespace Mednafen;

	extern VCE* vce;

	bool SetSoundRate(double rate);
	double GetSoundRate();
}
