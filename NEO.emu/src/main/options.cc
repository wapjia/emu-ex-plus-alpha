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
	#include <gngeo/roms.h>
	#include <gngeo/emu.h>
}

module system;

namespace EmuEx
{

void NeoSystem::setTimerIntOption()
{
	if(optionTimerInt == 2)
	{
		bool needsTimer = hasContent() && containsAny(contentDisplayName(),
			"Sidekicks 2", "Sidekicks 3", "Ultimate 11", "Neo-Geo Cup", "Spin Master", "Neo Turf Masters");
		if(needsTimer) log.info("auto enabled timer interrupt");
		conf.raster = needsTimer;
	}
	else
	{
		conf.raster = optionTimerInt;
	}
}

void NeoSystem::onOptionsLoaded()
{
	conf.system = SYSTEM(optionBIOSType.value());
	conf.country = COUNTRY(optionMVSCountry.value());
}

bool NeoSystem::resetSessionOptions(EmuApp &app)
{
	optionTimerInt.reset();
	setTimerIntOption();
	return true;
}

bool NeoSystem::readConfig(ConfigType type, MapIO &io, unsigned key)
{
	if(type == ConfigType::MAIN)
	{
		switch(key)
		{
			case CFGKEY_LIST_ALL_GAMES: return readOptionValue(io, optionListAllGames);
			case CFGKEY_BIOS_TYPE: return readOptionValue(io, optionBIOSType);
			case CFGKEY_MVS_COUNTRY: return readOptionValue(io, optionMVSCountry);
			case CFGKEY_CREATE_USE_CACHE: return readOptionValue(io, optionCreateAndUseCache);
			case CFGKEY_STRICT_ROM_CHECKING: return readOptionValue(io, optionStrictROMChecking);
		}
	}
	else if(type == ConfigType::SESSION)
	{
		switch(key)
		{
			case CFGKEY_TIMER_INT: return readOptionValue(io, optionTimerInt);
		}
	}
	return false;
}

void NeoSystem::writeConfig(ConfigType type, FileIO &io)
{
	if(type == ConfigType::MAIN)
	{
		writeOptionValueIfNotDefault(io, optionListAllGames);
		writeOptionValueIfNotDefault(io, optionBIOSType);
		writeOptionValueIfNotDefault(io, optionMVSCountry);
		writeOptionValueIfNotDefault(io, optionCreateAndUseCache);
		writeOptionValueIfNotDefault(io, optionStrictROMChecking);
	}
	else if(type == ConfigType::SESSION)
	{
		writeOptionValueIfNotDefault(io, optionTimerInt);
	}
}

}
