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

#include <emuframework/EmuApp.hh>
#include <emuframework/AudioOptionView.hh>
#include <emuframework/VideoOptionView.hh>
#include <emuframework/FilePathOptionView.hh>
#include <emuframework/UserPathSelectView.hh>
#include <emuframework/SystemActionsView.hh>
#include "Palette.hh"
#include "MainApp.hh"
#include <resample/resamplerinfo.h>
#include <imagine/logger/logger.h>

namespace EmuEx
{

using MainAppHelper = EmuAppHelperBase<MainApp>;

static constexpr size_t MAX_RESAMPLERS = 4;

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
		logMsg("%d resamplers", (int)ResamplerInfo::num());
		auto resamplers = std::min(ResamplerInfo::num(), MAX_RESAMPLERS);
		for(auto i : iotaCount(resamplers))
		{
			ResamplerInfo r = ResamplerInfo::get(i);
			logMsg("%zu %s", i, r.desc);
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
					logMsg("set cheats path:%s", path.data());
					system().cheatsDir = path;
					cheatsPath.compile(cheatsMenuName(appContext(), path));
				}), e);
		}
	};

public:
	CustomFilePathOptionView(ViewAttachParams attach): FilePathOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&cheatsPath);
	}
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

}
