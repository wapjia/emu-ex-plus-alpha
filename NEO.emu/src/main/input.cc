/*  This file is part of NEO.emu.

	NEO.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	NEO.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with NEO.emu.  If not, see <http://www.gnu.org/licenses/> */

module;
extern "C"
{
	#include <gngeo/memory.h>
	#include <gngeo/emu.h>
}

module system;

namespace EmuEx
{

void NeoSystem::handleInputAction(EmuApp *, InputAction a)
{
	auto player = a.flags.deviceId;
	bool isPushed = a.state == Input::Action::PUSHED;
	auto neoKey = NeoKey(a.code);
	if(a.code <= 8) // joystick bit index
	{
		auto &p = player ? memory.intern_p2 : memory.intern_p1;
		// Don't permit simultaneous left + right input, locks up Metal Slug 3
		if(isPushed && neoKey == NeoKey::Left)
		{
			p |= bit(to_underlying(NeoKey::Right) - 1);
		}
		else if(isPushed && neoKey == NeoKey::Right)
		{
			p |= bit(to_underlying(NeoKey::Left) - 1);
		}
		p = setOrClearBits(p, Uint8((bit(a.code - 1))), !isPushed);
	}
	else if(neoKey == NeoKey::Select)
	{
		constexpr unsigned coin1Bit = bit(0), coin2Bit = bit(1),
			select1Bit = bit(1), select2Bit = bit(3);
		if(conf.system == SYS_ARCADE)
		{
			unsigned bits = player ? coin2Bit : coin1Bit;
			memory.intern_coin = setOrClearBits(memory.intern_coin, (Uint8)bits, !isPushed);
		}
		else
		{
			// convert COIN to SELECT
			unsigned bits = player ? select2Bit : select1Bit;
			memory.intern_start = setOrClearBits(memory.intern_start, (Uint8)bits, !isPushed);
		}
	}
	else if(neoKey == NeoKey::Start)
	{
		constexpr unsigned start1Bit = bit(0), start2Bit = bit(2);
		unsigned bits = player ? start2Bit : start1Bit;
		memory.intern_start = setOrClearBits(memory.intern_start, (Uint8)bits, !isPushed);
	}
	else if(neoKey == NeoKey::TestSwitch)
	{
		if(isPushed)
			conf.test_switch = 1; // Test Switch is reset to 0 after every frame
		return;
	}
}

void NeoSystem::clearInputBuffers()
{
	memory.intern_coin = 0x7;
	memory.intern_start = 0x8F;
	memory.intern_p1 = 0xFF;
	memory.intern_p2 = 0xFF;
}



}
