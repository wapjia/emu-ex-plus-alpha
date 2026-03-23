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
#include <mednafen/types.h>
#include <mednafen/general.h>

module system;

extern "C++" void Lynx_SetLowpassFilter(bool);

namespace EmuEx
{

bool LynxSystem::readConfig(ConfigType type, MapIO &io, unsigned key)
{
	if(type == ConfigType::MAIN)
	{
		switch(key)
		{
			case CFGKEY_BIOS: return readStringOptionValue(io, biosPath);
			case CFGKEY_LOWPASS_FILTER: return readOptionValue(io, lowpassFilter);
			case CFGKEY_NO_MD5_FILENAMES: return readOptionValue(io, noMD5InFilenames);
		}
	}
	else if(type == ConfigType::SESSION)
	{
		switch(key)
		{
			case CFGKEY_LYNX_ROTATION: return readOptionValue(io, rotation, [](auto val)
				{
					return val <= lastEnum<LynxRotation>;
				});
		}
	}
	return false;
}

void LynxSystem::writeConfig(ConfigType type, FileIO &io)
{
	if(type == ConfigType::MAIN)
	{
		writeStringOptionValue(io, CFGKEY_BIOS, biosPath);
		writeOptionValueIfNotDefault(io, CFGKEY_LOWPASS_FILTER, lowpassFilter, false);
		writeOptionValueIfNotDefault(io, CFGKEY_NO_MD5_FILENAMES, noMD5InFilenames, false);
	}
	else if(type == ConfigType::SESSION)
	{
		if(rotation != LynxRotation::Auto)
			writeOptionValue(io, CFGKEY_LYNX_ROTATION, rotation);
	}
}

bool LynxSystem::resetSessionOptions(EmuApp &app)
{
	rotation = LynxRotation::Auto;
	return true;
}


void LynxSystem::setRotation(LynxRotation r)
{
	if(r == rotation || !hasContent())
		return;
	rotation = r;
	sessionOptionSet();
	auto &app = EmuApp::get(appContext());
	app.updateContentRotation();
}

void LynxSystem::setLowpassFilter(bool on)
{
	lowpassFilter = on;
	if(hasContent())
	{
		Lynx_SetLowpassFilter(on);
	}
}

}

extern "C++" namespace Mednafen
{

#define EMU_MODULE "lynx"

using namespace EmuEx;

uint64 MDFN_GetSettingUI(const char *name_)
{
	LynxSystem::log.error("unhandled settingUI:{}", name_);
	unreachable();
}

int64 MDFN_GetSettingI(const char *name_)
{
	std::string_view name{name_};
	if("filesys.state_comp_level" == name)
		return 6;
	LynxSystem::log.error("unhandled settingI:{}", name_);
	unreachable();
}

double MDFN_GetSettingF(const char *name)
{
	LynxSystem::log.error("unhandled settingF:{}", name);
	unreachable();
}

bool MDFN_GetSettingB(const char *name_)
{
	std::string_view name{name_};
	auto &sys = static_cast<LynxSystem&>(gSystem());
	if("cheats" == name)
		return false;
	if(EMU_MODULE".lowpass" == name)
		return sys.lowpassFilter;
	if("filesys.untrusted_fip_check" == name)
		return false;
	LynxSystem::log.error("unhandled settingB:{}", name_);
	unreachable();
}

std::string MDFN_GetSettingS(const char *name_)
{
	LynxSystem::log.error("unhandled settingS:{}", name_);
	unreachable();
}

std::string MDFN_MakeFName(MakeFName_Type type, int id1, const char *cd1)
{
	switch(type)
	{
		case MDFNMKF_STATE:
		case MDFNMKF_SAV:
		case MDFNMKF_SAVBACK:
			return savePathMDFN(static_cast<LynxApp&>(EmuEx::gApp()), id1, cd1);
		case MDFNMKF_FIRMWARE:
		{
			// lynx-specific
			auto &sys = static_cast<LynxSystem&>(gSystem());
			return std::string{sys.biosPath};
		}
		default: unreachable();
	}
}

}
