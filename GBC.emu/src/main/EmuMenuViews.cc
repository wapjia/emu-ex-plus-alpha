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

#include <resample/resamplerinfo.h>
import system;
import emuex;
import imagine;
import std;

namespace EmuEx
{

using namespace IG;
using MainAppHelper = EmuAppHelperBase<MainApp>;

constexpr size_t MAX_RESAMPLERS{4};

class CustomAudioOptionView : public AudioOptionView, public MainAppHelper
{
	using MainAppHelper::app;
	using MainAppHelper::system;

	StaticArrayList<TextMenuItem, MAX_RESAMPLERS> resamplerItem;

	MultiChoiceMenuItem resampler
	{
		"重采样器", attachParams(),
		system().optionAudioResampler.value(),
		resamplerItem
	};

public:
	CustomAudioOptionView(ViewAttachParams attach, EmuAudio& audio): AudioOptionView{attach, audio, true}
	{
		loadStockItems();
		GbcSystem::log.info("{} resamplers", ResamplerInfo::num());
		auto resamplers = std::min(ResamplerInfo::num(), MAX_RESAMPLERS);
		for(auto i: iotaCount(resamplers))
		{
			ResamplerInfo r = ResamplerInfo::get(i);
			GbcSystem::log.info("{} {}", i, r.desc);
			resamplerItem.emplace_back(r.desc, attachParams(),
				[this, i](){ system().optionAudioResampler = i; });
		}
		item.emplace_back(&resampler);
	}
};

class CustomVideoOptionView : public VideoOptionView, public MainAppHelper
{
	using MainAppHelper::system;

	TextMenuItem::SelectDelegate setGbPaletteDel()
	{
		return [this](TextMenuItem &item)
		{
			system().optionGBPal = item.id;
			system().applyGBPalette();
		};
	}

	TextMenuItem gbPaletteItem[13]
	{
		{"原画",   attachParams(), setGbPaletteDel(), {.id = 0}},
		{"棕色",      attachParams(), setGbPaletteDel(), {.id = 1}},
		{"红色",        attachParams(), setGbPaletteDel(), {.id = 2}},
		{"深棕色", attachParams(), setGbPaletteDel(), {.id = 3}},
		{"粉彩",     attachParams(), setGbPaletteDel(), {.id = 4}},
		{"橙色",     attachParams(), setGbPaletteDel(), {.id = 5}},
		{"黄色",     attachParams(), setGbPaletteDel(), {.id = 6}},
		{"蓝色",       attachParams(), setGbPaletteDel(), {.id = 7}},
		{"深蓝色",  attachParams(), setGbPaletteDel(), {.id = 8}},
		{"灰色",       attachParams(), setGbPaletteDel(), {.id = 9}},
		{"绿色",      attachParams(), setGbPaletteDel(), {.id = 10}},
		{"深绿色", attachParams(), setGbPaletteDel(), {.id = 11}},
		{"颜色反转",    attachParams(), setGbPaletteDel(), {.id = 12}},
	};

	MultiChoiceMenuItem gbPalette
	{
		"GB调色板", attachParams(),
		MenuId{system().optionGBPal},
		gbPaletteItem
	};

	BoolMenuItem fullSaturation
	{
		"饱和GBC颜色", attachParams(),
		(bool)system().optionFullGbcSaturation,
		[this](BoolMenuItem &item)
		{
			system().optionFullGbcSaturation = item.flipBoolValue(*this);
			if(system().hasContent())
			{
				system().refreshPalettes();
			}
		}
	};

public:
	CustomVideoOptionView(ViewAttachParams attach, EmuVideoLayer &layer): VideoOptionView{attach, layer, true}
	{
		loadStockItems();
		item.emplace_back(&systemSpecificHeading);
		item.emplace_back(&gbPalette);
		item.emplace_back(&fullSaturation);
	}
};

class ConsoleOptionView : public TableView, public MainAppHelper
{
	BoolMenuItem useBuiltinGBPalette
	{
		"使用内置GB调色板", attachParams(),
		(bool)system().optionUseBuiltinGBPalette,
		[this](BoolMenuItem &item)
		{
			system().sessionOptionSet();
			system().optionUseBuiltinGBPalette = item.flipBoolValue(*this);
			system().applyGBPalette();
		}
	};

	BoolMenuItem reportAsGba
	{
		"将硬件报告为GBA", attachParams(),
		system().optionReportAsGba,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().sessionOptionSet();
			system().optionReportAsGba = item.flipBoolValue(*this);
			app().promptSystemReloadDueToSetOption(attachParams(), e);
		}
	};

	std::array<MenuItem*, 2> menuItem
	{
		&useBuiltinGBPalette,
		&reportAsGba
	};

public:
	ConsoleOptionView(ViewAttachParams attach):
		TableView
		{
			"控制台设置",
			attach,
			menuItem
		}
	{}
};

class CustomSystemActionsView : public SystemActionsView
{
	TextMenuItem options
	{
		"控制台设置", attachParams(),
		[this](TextMenuItem &, View &, Input::Event e)
		{
			if(system().hasContent())
			{
				pushAndShow(makeView<ConsoleOptionView>(), e);
			}
		}
	};

public:
	CustomSystemActionsView(ViewAttachParams attach): SystemActionsView{attach, true}
	{
		item.emplace_back(&options);
		loadStandardItems();
	}
};

class CustomFilePathOptionView : public FilePathOptionView, public MainAppHelper
{
	using MainAppHelper::system;

	TextMenuItem cheatsPath
	{
		cheatsMenuName(appContext(), system().cheatsDir), attachParams(),
		[this](const Input::Event &e)
		{
			pushAndShow(makeViewWithName<UserPathSelectView>("Cheats", system().userPath(system().cheatsDir),
				[this](CStringView path)
				{
					GbcSystem::log.info("set cheats path:{}", path);
					system().cheatsDir = path;
					cheatsPath.compile(cheatsMenuName(appContext(), path));
				}), e);
		}
	};

public:
	CustomFilePathOptionView(ViewAttachParams attach): FilePathOptionView{attach, true}
	{
		loadStockItems();
		//item.emplace_back(&cheatsPath);//爱吾修改：去掉金手指文件路径
	}
};

class EditCheatView : public BaseEditCheatView
{
public:
	EditCheatView(ViewAttachParams attach, Cheat& cheat, BaseEditCheatsView& editCheatsView):
		BaseEditCheatView
		{
			"Edit Cheat",
			attach,
			cheat,
			editCheatsView
		},
		addGGGS
		{
			"Add Another Code", attach,
			[this](const Input::Event& e) { addNewCheatCode("Input xxxxxxxx (GS) or xxx-xxx-xxx (GG) code", e); }
		}
	{
		loadItems();
	}

	void loadItems()
	{
		codes.clear();
		for(auto& c: cheatPtr->codes)
		{
			codes.emplace_back("Code", c, attachParams(), [this, &c](const Input::Event& e)
			{
				pushAndShowNewCollectValueInputView<const char*, ScanValueMode::AllowBlank>(attachParams(), e,
					"Input xxxxxxxx (GS) or xxx-xxx-xxx (GG) code, or blank to delete", c,
					[this, &c](CollectTextInputView&, auto str) { return modifyCheatCode(c, {str}); });
			});
		};
		items.clear();
		items.emplace_back(&name);
		for(auto& c: codes)
		{
			items.emplace_back(&c);
		}
		items.emplace_back(&addGGGS);
		items.emplace_back(&remove);
	}

private:
	TextMenuItem addGGGS;
};

class EditCheatsView : public BaseEditCheatsView
{
public:
	EditCheatsView(ViewAttachParams attach, CheatsView& cheatsView):
		BaseEditCheatsView
		{
			attach,
			cheatsView,
			[this](ItemMessage msg) -> ItemReply
			{
				return msg.visit(overloaded
				{
					[&](const ItemsMessage&) -> ItemReply { return 1 + cheats.size(); },
					[&](const GetItemMessage& m) -> ItemReply
					{
						switch(m.idx)
						{
							case 0: return &addGGGS;
							default: return &cheats[m.idx - 1];
						}
					},
				});
			}
		},
		addGGGS
		{
			"Add Game Genie / GameShark Code", attach,
			[this](const Input::Event& e) { addNewCheat("Input xxxxxxxx (GS) or xxx-xxx-xxx (GG) code", e); }
		} {}

private:
	TextMenuItem addGGGS;
};

std::unique_ptr<View> EmuApp::makeCustomView(ViewAttachParams attach, ViewID id)
{
	switch(id)
	{
		case ViewID::VIDEO_OPTIONS: return std::make_unique<CustomVideoOptionView>(attach, videoLayer);
		case ViewID::AUDIO_OPTIONS: return std::make_unique<CustomAudioOptionView>(attach, audio);
		case ViewID::SYSTEM_ACTIONS: return std::make_unique<CustomSystemActionsView>(attach);
		case ViewID::FILE_PATH_OPTIONS: return std::make_unique<CustomFilePathOptionView>(attach);
		default: return nullptr;
	}
}

std::unique_ptr<View> AppMeta::makeEditCheatsView(ViewAttachParams attach, CheatsView& view) { return std::make_unique<EditCheatsView>(attach, view); }
std::unique_ptr<View> AppMeta::makeEditCheatView(ViewAttachParams attach, Cheat& c, BaseEditCheatsView& baseView) { return std::make_unique<EditCheatView>(attach, c, baseView); }

}
