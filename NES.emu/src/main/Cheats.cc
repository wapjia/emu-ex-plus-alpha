/*  This file is part of NES.emu.

	NES.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	NES.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with NES.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <imagine/gui/TextEntry.hh>
#include <imagine/util/string.h>
#include <imagine/util/format.hh>
#include <imagine/logger/logger.h>
#include <emuframework/Cheats.hh>
#include <emuframework/EmuApp.hh>
#include <emuframework/viewUtils.hh>
#include "EmuCheatViews.hh"
#include "MainSystem.hh"
#include <fceu/driver.h>
#include <fceu/cheat.h>
#include "MainSystem.hh"

void EncodeGG(char *str, int a, int v, int c);
void RebuildSubCheats();

namespace EmuEx
{

constexpr SystemLogger log{"NES.emu"};

static unsigned parseHex(const char* str) { return strtoul(str, nullptr, 16); }

constexpr bool isValidGGCodeLen(const char* str)
{
	return std::string_view{str}.size() == 6 || std::string_view{str}.size() == 8;
}

static void saveCheats()
{
	savecheats = 1;
	FCEU_FlushGameCheats(nullptr, 0, false);
}

static void syncCheats()
{
	saveCheats();
	RebuildSubCheats();
}

Cheat* NesSystem::newCheat(EmuApp& app, const char* name, CheatCodeDesc desc)
{
	auto cPtr = &static_cast<Cheat&>(cheats.emplace_back(name));
	if(!addCheatCode(app, cPtr, desc))
	{
		cheats.pop_back();
		return {};
	}
	log.info("added new cheat, {} total", cheats.size());
	return cPtr;
}

bool NesSystem::setCheatName(Cheat& c, const char* name)
{
	c.name = name;
	saveCheats();
	return true;
}

std::string_view NesSystem::cheatName(const Cheat& c) const { return c.name; }

void NesSystem::setCheatEnabled(Cheat& c, bool on)
{
	c.status = on;
	syncCheats();
}

bool NesSystem::isCheatEnabled(const Cheat& c) const { return c.status; }

bool NesSystem::addCheatCode(EmuApp& app, Cheat*& cheatPtr, CheatCodeDesc desc)
{
	if(desc.flags)
	{
		if(!isValidGGCodeLen(desc.str))
		{
			app.postMessage(true, "Invalid, must be 6 or 8 digits");
			return false;
		}
		uint16 a; uint8 v; int c;
		if(!FCEUI_DecodeGG(desc.str, &a, &v, &c))
		{
			app.postMessage(true, "Error decoding code");
			return false;
		}
		cheatPtr->codes.emplace_back(a, v, c, 1);
	}
	else
	{
		auto a = parseHex(desc.str);
		if(a > 0xFFFF)
		{
			app.postMessage(true, "Invalid address");
			return false;
		}
		cheatPtr->codes.emplace_back(a, 0, -1, 0);
	}
	syncCheats();
	return true;
}

bool NesSystem::modifyCheatCode(EmuApp& app, Cheat&, CheatCode& c, CheatCodeDesc desc)
{
	assert(desc.flags);
	if(!isValidGGCodeLen(desc.str))
	{
		app.postMessage(true, "Invalid, must be 6 or 8 digits");
		return false;
	}
	if(!FCEUI_DecodeGG(desc.str, &c.addr, &c.val, &c.compare))
	{
		app.postMessage(true, "Error decoding code");
		return false;
	}
	syncCheats();
	return true;
}

Cheat* NesSystem::removeCheatCode(Cheat& c, CheatCode& code)
{
	c.codes.erase(toIterator(c.codes, static_cast<CHEATCODE&>(code)));
	bool removedAllCodes = c.codes.empty();
	if(removedAllCodes)
		cheats.erase(toIterator(cheats, static_cast<CHEATF&>(c)));
	syncCheats();
	return removedAllCodes ? nullptr : &c;
}

bool NesSystem::removeCheat(Cheat& c)
{
	cheats.erase(toIterator(cheats, static_cast<CHEATF&>(c)));
	syncCheats();
	return true;
}

void NesSystem::forEachCheat(DelegateFunc<bool(Cheat&, std::string_view)> del)
{
	for(auto& c: cheats)
	{
		if(!del(static_cast<Cheat&>(c), std::string_view{c.name}))
			break;
	}
}

static std::string toGGString(const CheatCode& c)
{
	std::string code;
	code.resize(9);
	EncodeGG(code.data(), c.addr, c.val, c.compare);
	code.resize(8);
	return code;
}

void NesSystem::forEachCheatCode(Cheat& cheat, DelegateFunc<bool(CheatCode&, std::string_view)> del)
{
	for(auto& c_: cheat.codes)
	{
		auto& c = static_cast<CheatCode&>(c_);
		std::string code;
		if(c.type)
		{
			code = toGGString(c);
		}
		else
		{
			code = std::format("{:x}:{:x}", c.addr, c.val);
			if(c.compare != -1)
				code += std::format(":{:x}", c.compare);
		}
		del(c, std::string_view{code});
	}
}

static std::string codeCompareToString(int compare) { return compare != -1 ? std::format("{:x}", compare) : std::string{}; }

EditRamCheatView::EditRamCheatView(ViewAttachParams attach, Cheat& cheat_, CheatCode& code_, EditCheatView& editCheatView_):
	TableView
	{
		"Edit Memory Patch",
		attach,
		[this](ItemMessage msg) -> ItemReply
		{
			return msg.visit(overloaded
			{
				[&](const ItemsMessage &m) -> ItemReply { return 4u; },
				[&](const GetItemMessage &m) -> ItemReply
				{
					switch(m.idx)
					{
						case 0: return &addr;
						case 1: return &value;
						case 2: return &comp;
						case 3: return &remove;
						default: std::unreachable();
					}
				},
			});
		}
	},
	cheat{cheat_},
	code{code_},
	editCheatView{editCheatView_},
	addr
	{
		"Address",
		std::format("{:x}", code_.addr),
		attach,
		[this](const Input::Event& e)
		{
			pushAndShowNewCollectValueInputView<const char*>(attachParams(), e, "Input 4-digit hex", std::format("{:x}", code.addr),
				[this](CollectTextInputView&, auto str)
				{
					unsigned a = parseHex(str);
					if(a > 0xFFFF)
					{
						app().postMessage(true, "Invalid input");
						return false;
					}
					code.addr = a;
					syncCheats();
					addr.set2ndName(str);
					addr.place();
					editCheatView.loadItems();
					return true;
				});
		}
	},
	value
	{
		"Value",
		std::format("{:x}", code_.val),
		attach,
		[this](const Input::Event& e)
		{
			pushAndShowNewCollectValueInputView<const char*>(attachParams(), e, "Input 2-digit hex", std::format("{:x}", code.val),
				[this](CollectTextInputView&, auto str)
				{
					unsigned a = parseHex(str);
					if(a > 0xFF)
					{
						app().postMessage(true, "Invalid value");
						return false;
					}
					code.val = a;
					syncCheats();
					value.set2ndName(str);
					value.place();
					editCheatView.loadItems();
					return true;
				});
		}
	},
	comp
	{
		"Compare",
		codeCompareToString(code_.compare),
		attach,
		[this](const Input::Event& e)
		{
			pushAndShowNewCollectValueInputView<const char*, ScanValueMode::AllowBlank>(attachParams(), e, "Input 2-digit hex or blank", codeCompareToString(code.compare),
				[this](CollectTextInputView &, const char *str)
				{
					if(strlen(str))
					{
						unsigned a = parseHex(str);
						if(a > 0xFF)
						{
							app().postMessage(true, "Invalid value");
							return true;
						}
						code.compare = a;
						comp.set2ndName(str);
					}
					else
					{
						code.compare = -1;
						comp.set2ndName();
					}
					syncCheats();
					comp.place();
					editCheatView.loadItems();
					return true;
				});
		}
	},
	remove
	{
		"Delete", attach,
		[this](const Input::Event& e)
		{
			pushAndShowModal(makeView<YesNoAlertView>("Really delete this patch?",
				YesNoAlertView::Delegates{.onYes = [this]{ editCheatView.removeCheatCode(code); dismiss(); }}), e);
		}
	} {}

EditCheatView::EditCheatView(ViewAttachParams attach, Cheat& cheat, BaseEditCheatsView& editCheatsView):
	BaseEditCheatView
	{
		"Edit Cheat",
		attach,
		cheat,
		editCheatsView
	},
	addGG
	{
		"Add Another Code", attach,
		[this](const Input::Event& e) { addNewCheatCode("Input Game Genie code", e, 1); }
	},
	addRAM
	{
		"Add Another Patch", attach,
		[this](const Input::Event& e) { addNewCheatCode("Input RAM address hex", e, 0); }
	}
{
	loadItems();
}

void EditCheatView::loadItems()
{
	codes.clear();
	system().forEachCheatCode(*cheatPtr, [this](CheatCode& c, std::string_view code)
	{
		codes.emplace_back("Code", code, attachParams(), [this, &c](const Input::Event& e)
		{
			if(c.type)
			{
				pushAndShowNewCollectValueInputView<const char*, ScanValueMode::AllowBlank>(attachParams(), e, "Input Game Genie code", toGGString(c),
					[this, &c](CollectTextInputView&, auto str) { return modifyCheatCode(c, {str, 1}); });
			}
			else
			{
				pushAndShow(makeView<EditRamCheatView>(*cheatPtr, c, *this), e);
			}
		});
		return true;
	});
	items.clear();
	items.emplace_back(&name);
	for(auto& c: codes)
	{
		items.emplace_back(&c);
	}
	items.emplace_back(&addGG);
	items.emplace_back(&addRAM);
	items.emplace_back(&remove);
}

EditCheatsView::EditCheatsView(ViewAttachParams attach, CheatsView& cheatsView):
	BaseEditCheatsView
	{
		attach,
		cheatsView,
		[this](ItemMessage msg) -> ItemReply
		{
			return msg.visit(overloaded
			{
				[&](const ItemsMessage &m) -> ItemReply { return 2 + ::cheats.size(); },
				[&](const GetItemMessage &m) -> ItemReply
				{
					switch(m.idx)
					{
						case 0: return &addGG;
						case 1: return &addRAM;
						default: return &cheats[m.idx - 2];
					}
				},
			});
		}
	},
	addGG
	{
		"Add Game Genie Code", attachParams(),
		[this](const Input::Event& e) { addNewCheat("Input Game Genie code", e, 1); }
	},
	addRAM
	{
		"Add Memory Patch", attachParams(),
		[this](const Input::Event& e) { addNewCheat("Input RAM Address Hex", e, 0); }
	} {}

//region 爱吾修改
static std::vector<std::string> split(std::string s,char ch)
{
    int start=0;
    int len=0;
    std::vector<std::string> ret;
    for(int i=0;i<s.length();i++){
        if(s[i]==ch){
            ret.push_back(s.substr(start,len));
            start=i+1;
            len=0;
        }
        else{
            len++;
        }
    }
    if(start<s.length())
        ret.push_back(s.substr(start,len));
    return ret;
}
void NesSystem::setCheatListAiWu(const std::list<std::string>& cheatList)
{
    if(!hasContent())
        return;
    //先清空金手指
	cheats.clear();
    //再添加新的金手指
	for (const std::string& cheat : cheatList)
	{
        //先判断是不是RAM
        const char *str = cheat.c_str();
		bool isRam = std::string_view{str}.find('-') != std::string_view::npos;
        if(isRam){//RAM
            std::vector<std::string> strs = split(cheat,'-');
        	if (strs.empty())
        	{
        		continue;
        	}
            //地址
            unsigned address = parseHex(strs[0].c_str());
            if(address > 0xFFFF)
            {
                continue;
            }
            //比较值（可选）
            int compare = -1;
            if(strs.size() == 3)
            {
                compare = parseHex(strs[1].c_str());
                if(compare > 0xFF)
                {
                    continue;
                }
            }
            //值
            int lastIndex = strs.size()-1;
            unsigned value = parseHex(strs[lastIndex].c_str());
            if(value > 0xFF)
            {
                continue;
            }
        	auto cheatPtr = &static_cast<Cheat&>(cheats.emplace_back("RAM Cheat"));
        	cheatPtr->codes.emplace_back(address, value, compare, 0);
			cheatPtr->status = true;
        } else {//GG
            if(!isValidGGCodeLen(str))
            {
                continue;
            }
            uint16 address; uint8 value; int compare;
            if(!FCEUI_DecodeGG(str, &address, &value, &compare))
            {
                continue;
            }
        	auto cheatPtr = &static_cast<Cheat&>(cheats.emplace_back("GG Cheat"));
        	cheatPtr->codes.emplace_back(address, value, compare, 1);
			cheatPtr->status = true;
        }
    }
	log.info("new cheat count:{}", cheats.size());
    //刷新金手指
	RebuildSubCheats();
}
//endregion
}
