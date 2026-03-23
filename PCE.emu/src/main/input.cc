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

module system;

namespace EmuEx
{

void PceSystem::handleInputAction(EmuApp *, InputAction a)
{
	auto player = a.flags.deviceId;
	assume(player < AppMeta::maxPlayers);
	inputBuff[player] = setOrClearBits(inputBuff[player], bit(a.code - 1), a.state == Input::Action::PUSHED);
}

void PceSystem::clearInputBuffers()
{
	inputBuff = {};
	if(option6BtnPad)
	{
		for(auto &padData : std::span{inputBuff.data(), 2})
			padData = bit(12);
	}
}

void set6ButtonPadEnabled(EmuApp &app, bool on)
{
	if(on)
	{
		app.unsetDisabledInputKeys();
	}
	else
	{
		static constexpr std::array extraCodes{KeyCode(PceKey::III), KeyCode(PceKey::IV), KeyCode(PceKey::V), KeyCode(PceKey::VI)};
		app.setDisabledInputKeys(extraCodes);
	}
}

}
