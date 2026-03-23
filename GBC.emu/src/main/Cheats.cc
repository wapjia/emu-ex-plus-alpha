/*  This file is part of GBC.emu.

	GBC.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	GBC.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with GBC.emu.  If not, see <http://www.gnu.org/licenses/> */

module;
#include <gambatte.h>

module system;

namespace EmuEx
{

static bool strIsGGCode(const char *str)
{
	int hex;
	return std::strlen(str) == 11 &&
		sscanf(str, "%1x%1x%1x-%1x%1x%1x-%1x%1x%1x",
			&hex, &hex, &hex, &hex, &hex, &hex, &hex, &hex, &hex) == 9;
}

static bool strIsGSCode(const char *str)
{
	int hex;
	return std::strlen(str) == 8 &&
		sscanf(str, "%1x%1x%1x%1x%1x%1x%1x%1x",
			&hex, &hex, &hex, &hex, &hex, &hex, &hex, &hex) == 8;
}

static bool strIsCode(const char *str)
{
	return strIsGGCode(str) || strIsGSCode(str);
}

void GbcSystem::applyCheats()
{
	if(!hasContent())
		return;
	std::string ggCodeStr, gsCodeStr;
	for(auto &e : cheatList)
	{
		if(!e.on)
			continue;
		for(const auto& c: e.codes)
		{
			std::string &codeStr = std::string_view{c}.contains('-') ? ggCodeStr : gsCodeStr;
			if(codeStr.size())
				codeStr += ';';
			codeStr += c;
		}
	}
	gbEmu.setGameGenie(ggCodeStr);
	gbEmu.setGameShark(gsCodeStr);
	if(ggCodeStr.size())
		log.info("set GG codes:{}", ggCodeStr);
	if(gsCodeStr.size())
		log.info("set GS codes:{}", gsCodeStr);
}

void GbcSystem::writeCheatFile()
{
	auto ctx = appContext();
	auto path = userFilePath(cheatsDir, ".gbcht");

	if(!cheatList.size())
	{
		log.info("deleting cheats file:{}", path);
		ctx.removeFileUri(path);
		return;
	}

	auto file = ctx.openFileUri(path, OpenFlags::testNewFile());
	if(!file)
	{
		log.error("error creating cheats file:{}", path);
		return;
	}
	log.info("writing cheats file:{}", path);

	int version = 1;
	file.put(int8_t(version));
	file.put(uint16_t(cheatList.size()));
	for(auto &e : cheatList)
	{
		file.put(uint8_t(e.on));
		writeSizedData<uint16_t>(file, e.name);
		file.put(uint16_t(e.codes.size()));
		for(auto& code: e.codes)
		{
			writeSizedData<uint8_t>(file, code);
		}
	}
}

void GbcSystem::readCheatFile()
{
	auto path = userFilePath(cheatsDir, ".gbcht");
	auto file = appContext().openFileUri(path, {.test = true, .accessHint = IOAccessHint::All});
	if(!file)
	{
		return;
	}
	log.info("reading cheats file:{}", path);

	const auto version = file.get<int8_t>();
	if(version > 1)
	{
		log.info("skipping due to version code:{}", version);
		return;
	}
	auto size = file.get<uint16_t>();
	cheatList.reserve(size);
	for(auto _: iotaCount(size))
	{
		Cheat cheat;
		cheat.on = file.get<uint8_t>();
		readSizedData<uint16_t>(file, cheat.name);
		if(version == 0)
		{
			cheat.codes.resize(1);
			readSizedData<uint8_t>(file, cheat.codes[0]);
		}
		else
		{
			auto codes = file.get<uint16_t>();
			cheat.codes.resize(codes);
			for(auto& c: cheat.codes)
			{
				readSizedData<uint8_t>(file, c);
			}
		}
		if(cheat.codes.size()) // ignore codes with blank names
		{
			cheatList.push_back(cheat);
		}
	}
}

Cheat* GbcSystem::newCheat(EmuApp& app, const char* name, CheatCodeDesc desc)
{
	if(!strIsCode(desc.str))
	{
		app.postMessage(true, "Invalid format");
		return {};
	}
	auto& c = cheatList.emplace_back(Cheat{.name = name});
	c.codes.emplace_back(toUpperCase<CheatCode>(desc.str));
	log.info("added new cheat, {} total", cheatList.size());
	applyCheats();
	writeCheatFile();
	return &c;
}

bool GbcSystem::setCheatName(Cheat& c, const char* name)
{
	c.name = name;
	writeCheatFile();
	return true;
}

std::string_view GbcSystem::cheatName(const Cheat& c) const { return c.name; }

void GbcSystem::setCheatEnabled(Cheat& c, bool on)
{
	c.on = on;
	writeCheatFile();
	applyCheats();
}

bool GbcSystem::isCheatEnabled(const Cheat& c) const { return c.on; }

bool GbcSystem::addCheatCode(EmuApp& app, Cheat*& cheatPtr, CheatCodeDesc desc)
{
	if(!strIsCode(desc.str))
	{
		app.postMessage(true, "Invalid format");
		return false;
	}
	cheatPtr->codes.emplace_back(toUpperCase<CheatCode>(desc.str));
	writeCheatFile();
	applyCheats();
	return true;
}

bool GbcSystem::modifyCheatCode(EmuApp& app, Cheat&, CheatCode& code, CheatCodeDesc desc)
{
	if(!strIsCode(desc.str))
	{
		app.postMessage(true, "Invalid format");
		return false;
	}
	code = toUpperCase<CheatCode>(desc.str);
	writeCheatFile();
	applyCheats();
	return true;
}

Cheat* GbcSystem::removeCheatCode(Cheat& c, CheatCode& code)
{
	eraseFirst(c.codes, code);
	bool removedAllCodes = c.codes.empty();
	if(removedAllCodes)
		eraseFirst(cheatList, c);
	writeCheatFile();
	applyCheats();
	return removedAllCodes ? nullptr : &c;
}

bool GbcSystem::removeCheat(Cheat& c)
{
	eraseFirst(cheatList, c);
	writeCheatFile();
	applyCheats();
	return true;
}

void GbcSystem::forEachCheat(DelegateFunc<bool(Cheat&, std::string_view)> del)
{
	for(auto& c: cheatList)
	{
		if(!del(c, std::string_view{c.name}))
			break;
	}
}

void GbcSystem::forEachCheatCode(Cheat& cheat, DelegateFunc<bool(CheatCode&, std::string_view)> del)
{
	for(auto& c: cheat.codes)
	{
		if(!del(c, std::string_view{c}))
			break;
	}
}

//region 爱吾修改
void GbcSystem::setCheatListAiWu(const std::list<std::string>& cheatList)
{
	if(!hasContent())
		return;
	std::string ggCodeStr, gsCodeStr;
	for (const std::string& cheat : cheatList)
	{
		std::string& codeStr = (cheat.find('-') != std::string::npos) ? ggCodeStr : gsCodeStr;
		if(!codeStr.empty())
			codeStr += ";";
		codeStr += cheat;
	}
	gbEmu.setGameGenie(ggCodeStr);
	gbEmu.setGameShark(gsCodeStr);
	if(ggCodeStr.size())
		log.info("set GG codes:{}", ggCodeStr);
	if(gsCodeStr.size())
		log.info("set GS codes:{}", gsCodeStr);
}
//endregion
}
