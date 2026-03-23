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

#include <fceu/driver.h>
#include <fceu/fds.h>
#include <fceu/sound.h>
#include <fceu/fceu.h>
#include <fceu/cheat.h>
import system;
import emuex;
import imagine;
import std;

namespace EmuEx
{

using namespace IG;
using MainAppHelper = EmuAppHelperBase<MainApp>;

class ConsoleOptionView : public TableView, public MainAppHelper
{
	BoolMenuItem fourScore
	{
		"4玩家模式", attachParams(),
		(bool)system().optionFourScore,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().sessionOptionSet();
			system().optionFourScore = item.flipBoolValue(*this);
			system().setupNESFourScore();
		}
	};

	static uint16_t packInputEnums(ESI port1, ESI port2)
	{
		return (uint16_t)port1 | ((uint16_t)port2 << 8);
	}

	static std::pair<ESI, ESI> unpackInputEnums(uint16_t packed)
	{
		return {ESI(packed & 0xFF), ESI(packed >> 8)};
	}

	TextMenuItem inputPortsItem[4]
	{
		{"自动",          attachParams(), {.id = packInputEnums(SI_UNSET, SI_UNSET)}},
		{"Gamepads",      attachParams(), {.id = packInputEnums(SI_GAMEPAD, SI_GAMEPAD)}},
		{"Gun (2P, NES)", attachParams(), {.id = packInputEnums(SI_GAMEPAD, SI_ZAPPER)}},
		{"Gun (1P, VS)",  attachParams(), {.id = packInputEnums(SI_ZAPPER, SI_GAMEPAD)}},
	};

	MultiChoiceMenuItem inputPorts
	{
		"输入端口", attachParams(),
		MenuId{packInputEnums(system().inputPort1.value(), system().inputPort2.value())},
		inputPortsItem,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				system().sessionOptionSet();
				auto [port1, port2] = unpackInputEnums(item.id);
				system().inputPort1 = port1;
				system().inputPort2 = port2;
				system().setupNESInputPorts();
			}
		}
	};

	BoolMenuItem fcMic
	{
		"P2作为麦克风启动", attachParams(),
		replaceP2StartWithMicrophone,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().sessionOptionSet();
			replaceP2StartWithMicrophone = item.flipBoolValue(*this);
		}
	};

	TextMenuItem videoSystemItem[4]
	{
		{"自动",  attachParams(), {.id = 0}},
		{"NTSC",  attachParams(), {.id = 1}},
		{"PAL",   attachParams(), {.id = 2}},
		{"Dendy", attachParams(), {.id = 3}},
	};

	MultiChoiceMenuItem videoSystem
	{
		"系统", attachParams(),
		MenuId{system().optionVideoSystem},
		videoSystemItem,
		{
			.onSetDisplayString = [](auto idx, Gfx::Text &t)
			{
				if(idx == 0)
				{
					t.resetString(dendy ? "Dendy" : pal_emulation ? "PAL" : "NTSC");
					return true;
				}
				return false;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item, Input::Event e)
			{
				system().sessionOptionSet();
				system().optionVideoSystem = item.id;
				setRegion(item.id, system().optionDefaultVideoSystem, system().autoDetectedRegion);
				app().promptSystemReloadDueToSetOption(attachParams(), e);
			}
		},
	};

	BoolMenuItem compatibleFrameskip
	{
		"跳帧模式", attachParams(),
		(bool)system().optionCompatibleFrameskip,
		"快速", "兼容",
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			if(!item.boolValue())
			{
				app().pushAndShowModalView(makeView<YesNoAlertView>(
					"如果游戏在快进/跳帧时出现故障，请使用兼容模式，但会增加CPU使用率。",
					YesNoAlertView::Delegates
					{
						.onYes = [this, &item]
						{
							system().sessionOptionSet();
							system().optionCompatibleFrameskip = item.flipBoolValue(*this);
						}
					}), e);
			}
			else
			{
				system().sessionOptionSet();
				system().optionCompatibleFrameskip = item.flipBoolValue(*this);
			}
		}
	};

	TextHeadingMenuItem videoHeading{"视频", attachParams()};

	static uint16_t packVideoLines(uint8_t start, uint8_t total)
	{
		return (uint16_t)start | ((uint16_t)total << 8);
	}

	static std::pair<uint8_t, uint8_t> unpackVideoLines(uint16_t packed)
	{
		return {uint8_t(packed & 0xFF), uint8_t(packed >> 8)};
	}

	TextMenuItem visibleVideoLinesItem[4]
	{
		{"8+224", attachParams(), {.id = packVideoLines(8, 224)}},
		{"8+232", attachParams(), {.id = packVideoLines(8, 232)}},
		{"0+232", attachParams(), {.id = packVideoLines(0, 232)}},
		{"0+240", attachParams(), {.id = packVideoLines(0, 240)}},
	};

	MultiChoiceMenuItem visibleVideoLines
	{
		"可见视频线", attachParams(),
		MenuId{packVideoLines(system().optionStartVideoLine, system().optionVisibleVideoLines)},
		visibleVideoLinesItem,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				auto [startLine, lines] = unpackVideoLines(item.id);
				system().sessionOptionSet();
				system().optionStartVideoLine = startLine;
				system().optionVisibleVideoLines = lines;
				system().updateVideoPixmap(app().video, system().optionHorizontalVideoCrop, system().optionVisibleVideoLines);
				system().renderFramebuffer(app().video);
				app().viewController().placeEmuViews();
			}
		}
	};

	BoolMenuItem horizontalVideoCrop
	{
		"在侧面裁剪8个像素", attachParams(),
		(bool)system().optionHorizontalVideoCrop,
		[this](BoolMenuItem &item)
		{
			system().sessionOptionSet();
			system().optionHorizontalVideoCrop = item.flipBoolValue(*this);
			system().updateVideoPixmap(app().video, system().optionHorizontalVideoCrop, system().optionVisibleVideoLines);
			system().renderFramebuffer(app().video);
			app().viewController().placeEmuViews();
		}
	};

	TextHeadingMenuItem overclocking{"超频", attachParams()};

	BoolMenuItem overclockingEnabled
	{
		"启用", attachParams(),
		overclock_enabled,
		[this](BoolMenuItem &item)
		{
			system().sessionOptionSet();
			overclock_enabled = item.flipBoolValue(*this);
		}
	};

	DualTextMenuItem extraLines
	{
		"每帧额外行数", std::to_string(postrenderscanlines), attachParams(),
		[this](const Input::Event &e)
		{
			pushAndShowNewCollectValueRangeInputView<int, 0, maxExtraLinesPerFrame>(attachParams(), e,
				"输入0到30000", std::to_string(postrenderscanlines),
				[this](CollectTextInputView&, auto val)
				{
					system().sessionOptionSet();
					postrenderscanlines = val;
					extraLines.set2ndName(std::to_string(val));
					return true;
				});
		}
	};

	DualTextMenuItem vblankMultipler
	{
		"垂直空白行倍数", std::to_string(vblankscanlines), attachParams(),
		[this](const Input::Event &e)
		{
			pushAndShowNewCollectValueRangeInputView<int, 0, maxVBlankMultiplier>(attachParams(), e,
				"输入0到16", std::to_string(vblankscanlines),
				[this](CollectTextInputView&, auto val)
				{
					system().sessionOptionSet();
					vblankscanlines = val;
					vblankMultipler.set2ndName(std::to_string(val));
					return true;
				});
		}
	};

	std::array<MenuItem*, 12> menuItem
	{
		&inputPorts,
		&fourScore,
		&fcMic,
		&compatibleFrameskip,
		&videoHeading,
		&videoSystem,
		&visibleVideoLines,
		&horizontalVideoCrop,
		&overclocking,
		&overclockingEnabled,
		&extraLines,
		&vblankMultipler,
	};

public:
	ConsoleOptionView(ViewAttachParams attach):
		TableView
		{
			"控制台设置",
			attach,
			menuItem
		} {}
};

class CustomVideoOptionView : public VideoOptionView, public MainAppHelper
{
	using MainAppHelper::app;
	using MainAppHelper::system;

	BoolMenuItem spriteLimit
	{
		"精灵限制", attachParams(),
		(bool)system().optionSpriteLimit,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().optionSpriteLimit = item.flipBoolValue(*this);
			FCEUI_DisableSpriteLimitation(!system().optionSpriteLimit);
		}
	};

	TextMenuItem videoSystemItem[4]
	{
		{"自动", attachParams(), [this](){ system().optionDefaultVideoSystem = 0; }},
		{"NTSC", attachParams(), [this](){ system().optionDefaultVideoSystem = 1; }},
		{"PAL", attachParams(), [this](){ system().optionDefaultVideoSystem = 2; }},
		{"Dendy", attachParams(), [this](){ system().optionDefaultVideoSystem = 3; }},
	};

	MultiChoiceMenuItem videoSystem
	{
		"默认视频系统", attachParams(),
		system().optionDefaultVideoSystem.value(),
		videoSystemItem
	};

	static constexpr auto digitalPrimePalPath = "Digital Prime (FBX).pal";
	static constexpr auto smoothPalPath = "Smooth V2 (FBX).pal";
	static constexpr auto magnumPalPath = "Magnum (FBX).pal";
	static constexpr auto classicPalPath = "Classic (FBX).pal";
	static constexpr auto wavebeamPalPath = "Wavebeam.pal";
	static constexpr auto lightfulPalPath = "Lightful.pal";
	static constexpr auto palightfulPalPath = "Palightful.pal";
	static constexpr auto fiveRealityPalPath = "Five Reality.pal";

	void setPalette(ApplicationContext ctx, CStringView palPath)
	{
		if(palPath.size())
			system().defaultPalettePath = palPath;
		else
			system().defaultPalettePath = {};
		system().setDefaultPalette(ctx, palPath);
		auto &app = EmuApp::get(ctx);
		app.renderSystemFramebuffer();
	}

	constexpr size_t defaultPaletteCustomFileIdx()
	{
		return lastIndex(defaultPalItem);
	}

	TextMenuItem defaultPalItem[8]
	{
		{"FCEUX",               attachParams(), [this]() { setPalette(appContext(), ""); }},
		{"Digital Prime (FBX)", attachParams(), [this]() { setPalette(appContext(), digitalPrimePalPath); }},
		{"Smooth V2 (FBX)",     attachParams(), [this]() { setPalette(appContext(), smoothPalPath); }},
		{"Magnum (FBX)",        attachParams(), [this]() { setPalette(appContext(), magnumPalPath); }},
		{"Classic (FBX)",       attachParams(), [this]() { setPalette(appContext(), classicPalPath); }},
		{"Wavebeam",            attachParams(), [this]() { setPalette(appContext(), wavebeamPalPath); }},
		{"Five Reality",        attachParams(), [this]() { setPalette(appContext(), fiveRealityPalPath); }},
		{"自定义文件", attachParams(), [this](Input::Event e)
			{
				auto fsFilter = [](std::string_view name) { return endsWithAnyCaseless(name, ".pal"); };
				auto fPicker = makeView<FilePicker>(FSPicker::Mode::FILE, fsFilter, e, false);
				fPicker->setOnSelectPath(
					[this](FSPicker &picker, CStringView path, std::string_view name, Input::Event)
					{
						setPalette(appContext(), path.data());
						defaultPal.setSelected(defaultPaletteCustomFileIdx());
						dismissPrevious();
						picker.dismiss();
					});
				fPicker->setPath(app().contentSearchPath, e);
				app().pushAndShowModalView(std::move(fPicker), e);
				return false;
			}},
	};

	MultiChoiceMenuItem defaultPal
	{
		"默认调色板", attachParams(),
		[this]()
		{
			if(system().defaultPalettePath.empty()) return 0;
			if(system().defaultPalettePath == digitalPrimePalPath) return 1;
			if(system().defaultPalettePath == smoothPalPath) return 2;
			if(system().defaultPalettePath == magnumPalPath) return 3;
			if(system().defaultPalettePath == classicPalPath) return 4;
			if(system().defaultPalettePath == wavebeamPalPath) return 5;
			if(system().defaultPalettePath == fiveRealityPalPath) return 6;
			return (int)defaultPaletteCustomFileIdx();
		}(),
		defaultPalItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(idx == defaultPaletteCustomFileIdx())
				{
					t.resetString(withoutDotExtension(appContext().fileUriDisplayName(system().defaultPalettePath)));
					return true;
				}
				return false;
			}
		},
	};

	TextMenuItem visibleVideoLinesItem[4]
	{
		{"8+224", attachParams(), setVisibleVideoLinesDel(8, 224)},
		{"8+232", attachParams(), setVisibleVideoLinesDel(8, 232)},
		{"0+232", attachParams(), setVisibleVideoLinesDel(0, 232)},
		{"0+240", attachParams(), setVisibleVideoLinesDel(0, 240)},
	};

	MultiChoiceMenuItem visibleVideoLines
	{
		"默认可见视频线", attachParams(),
		[this]()
		{
			switch(system().optionDefaultVisibleVideoLines)
			{
				default: return 0;
				case 232: return system().optionDefaultStartVideoLine == 8 ? 1 : 2;
				case 240: return 3;
			}
		}(),
		visibleVideoLinesItem
	};

	TextMenuItem::SelectDelegate setVisibleVideoLinesDel(uint8_t startLine, uint8_t lines)
	{
		return [this, startLine, lines]()
		{
			system().optionDefaultStartVideoLine = startLine;
			system().optionDefaultVisibleVideoLines = lines;
		};
	}

	BoolMenuItem correctLineAspect
	{
		"正确的线条比例", attachParams(),
		(bool)system().optionCorrectLineAspect,
		[this](BoolMenuItem &item)
		{
			system().optionCorrectLineAspect = item.flipBoolValue(*this);
			app().viewController().placeEmuViews();
		}
	};

public:
	CustomVideoOptionView(ViewAttachParams attach, EmuVideoLayer &layer): VideoOptionView{attach, layer, true}
	{
		loadStockItems();
		item.emplace_back(&systemSpecificHeading);
		item.emplace_back(&defaultPal);
		item.emplace_back(&videoSystem);
		item.emplace_back(&spriteLimit);
		item.emplace_back(&visibleVideoLines);
		item.emplace_back(&correctLineAspect);
	}
};

class CustomAudioOptionView : public AudioOptionView, public MainAppHelper
{
	using MainAppHelper::system;

	void setQuality(int quaility)
	{
		system().optionSoundQuality = quaility;
		FCEUI_SetSoundQuality(quaility);
	}

	TextMenuItem qualityItem[3]
	{
		{"正常", attachParams(), [this](){ setQuality(0); }},
		{"高", attachParams(), [this]() { setQuality(1); }},
		{"最高", attachParams(), [this]() { setQuality(2); }}
	};

	MultiChoiceMenuItem quality
	{
		"模拟质量", attachParams(),
		system().optionSoundQuality.value(),
		qualityItem
	};

	BoolMenuItem lowPassFilter
	{
		"低通滤波器", attachParams(),
		(bool)FSettings.lowpass,
		[this](BoolMenuItem &item)
		{
			FCEUI_SetLowPass(item.flipBoolValue(*this));
		}
	};

	BoolMenuItem swapDutyCycles
	{
		"交换占空比", attachParams(),
		swapDuty,
		[this](BoolMenuItem &item)
		{
			swapDuty = item.flipBoolValue(*this);
		}
	};

	TextHeadingMenuItem mixer{"混合器", attachParams()};

	BoolMenuItem squareWave1
	{
		"Square Wave #1", attachParams(),
		(bool)FSettings.Square1Volume,
		[this](BoolMenuItem &item)
		{
			FSettings.Square1Volume = item.flipBoolValue(*this) ? 256 : 0;
		}
	};

	BoolMenuItem squareWave2
	{
		"Square Wave #2", attachParams(),
		(bool)FSettings.Square2Volume,
		[this](BoolMenuItem &item)
		{
			FSettings.Square2Volume = item.flipBoolValue(*this) ? 256 : 0;
		}
	};

	BoolMenuItem triangleWave1
	{
		"Triangle Wave", attachParams(),
		(bool)FSettings.TriangleVolume,
		[this](BoolMenuItem &item)
		{
			FSettings.TriangleVolume = item.flipBoolValue(*this) ? 256 : 0;
		}
	};

	BoolMenuItem noise
	{
		"Noise", attachParams(),
		(bool)FSettings.NoiseVolume,
		[this](BoolMenuItem &item)
		{
			FSettings.NoiseVolume = item.flipBoolValue(*this) ? 256 : 0;
		}
	};

	BoolMenuItem dpcm
	{
		"DPCM", attachParams(),
		(bool)FSettings.PCMVolume,
		[this](BoolMenuItem &item)
		{
			FSettings.PCMVolume = item.flipBoolValue(*this) ? 256 : 0;
		}
	};

public:
	CustomAudioOptionView(ViewAttachParams attach, EmuAudio& audio): AudioOptionView{attach, audio, true}
	{
		loadStockItems();
		item.emplace_back(&quality);
		item.emplace_back(&lowPassFilter);
		item.emplace_back(&swapDutyCycles);
		item.emplace_back(&mixer);
		item.emplace_back(&squareWave1);
		item.emplace_back(&squareWave2);
		item.emplace_back(&triangleWave1);
		item.emplace_back(&noise);
		item.emplace_back(&dpcm);
	}
};

class CustomFilePathOptionView : public FilePathOptionView, public MainAppHelper
{
	using MainAppHelper::app;
	using MainAppHelper::system;

	TextMenuItem cheatsPath
	{
		cheatsMenuName(appContext(), system().cheatsDir), attachParams(),
		[this](const Input::Event &e)
		{
			pushAndShow(makeViewWithName<UserPathSelectView>("Cheats", system().userPath(system().cheatsDir),
				[this](CStringView path)
				{
					NesSystem::log.info("set cheats path:{}", path);
					system().cheatsDir = path;
					cheatsPath.compile(cheatsMenuName(appContext(), path));
				}), e);
		}
	};

	TextMenuItem patchesPath
	{
		patchesMenuName(appContext(), system().patchesDir), attachParams(),
		[this](const Input::Event &e)
		{
			pushAndShow(makeViewWithName<UserPathSelectView>("补丁", system().userPath(system().patchesDir),
				[this](CStringView path)
				{
					NesSystem::log.info("set patches path:{}", path);
					system().patchesDir = path;
					patchesPath.compile(patchesMenuName(appContext(), path));
				}), e);
		}
	};

	TextMenuItem palettesPath
	{
		palettesMenuName(appContext(), system().palettesDir), attachParams(),
		[this](const Input::Event &e)
		{
			pushAndShow(makeViewWithName<UserPathSelectView>("调色板", system().userPath(system().palettesDir),
				[this](CStringView path)
				{
					NesSystem::log.info("set palettes path:{}", path);
					system().palettesDir = path;
					palettesPath.compile(palettesMenuName(appContext(), path));
				}), e);
		}
	};

	TextMenuItem fdsBios
	{
		biosMenuEntryStr(system().fdsBiosPath), attachParams(),
		[this](TextMenuItem &, View &, Input::Event e)
		{
			pushAndShow(makeViewWithName<DataFileSelectView<>>("磁碟机BIOS",
				app().validSearchPath(FS::dirnameUri(system().fdsBiosPath)),
				[this](CStringView path, FS::file_type type)
				{
					//爱吾修改：增加默认磁碟机Bios(Disk System BIOS)路径
					if(path.empty()){
						system().fdsBiosPath = std::string(FS::pathString(appContext().supportPath(), "Disksys.rom"));
					}else{
						system().fdsBiosPath = path;
					}
					NesSystem::log.info("set fds bios:{}", system().fdsBiosPath);
					fdsBios.compile(biosMenuEntryStr(system().fdsBiosPath));
					return true;
				}, hasFDSBIOSExtension), e);
		}
	};

	std::string biosMenuEntryStr(CStringView path) const
	{
		return std::format("磁碟机BIOS: {}", appContext().fileUriDisplayName(path));
	}

public:
	CustomFilePathOptionView(ViewAttachParams attach): FilePathOptionView{attach, true}
	{
		loadStockItems();
		//item.emplace_back(&cheatsPath);//去掉金手指文件路径
		item.emplace_back(&patchesPath);
		item.emplace_back(&palettesPath);
		item.emplace_back(&fdsBios);
	}
};

class FDSControlView : public TableView, public MainAppHelper
{
private:
	static constexpr unsigned DISK_SIDES = 4;
	TextMenuItem setSide[DISK_SIDES]
	{
		{
			"Set Disk 1 Side A", attachParams(),
			[this](View &view, Input::Event e)
			{
				FCEU_FDSSetDisk(0, static_cast<NesSystemHolder&>(system()));
				view.dismiss();
			}
		},
		{
			"Set Disk 1 Side B", attachParams(),
			[this](View &view, Input::Event e)
			{
				FCEU_FDSSetDisk(1, static_cast<NesSystemHolder&>(system()));
				view.dismiss();
			}
		},
		{
			"Set Disk 2 Side A", attachParams(),
			[this](View &view, Input::Event e)
			{
				FCEU_FDSSetDisk(2, static_cast<NesSystemHolder&>(system()));
				view.dismiss();
			}
		},
		{
			"Set Disk 2 Side B", attachParams(),
			[this](View &view, Input::Event e)
			{
				FCEU_FDSSetDisk(3, static_cast<NesSystemHolder&>(system()));
				view.dismiss();
			}
		}
	};

	TextMenuItem insertEject
	{
		"弹出", attachParams(),
		[](View& view)
		{
			if(FCEU_FDSInserted())
			{
				FCEU_FDSInsert();
				view.dismiss();
			}
		}
	};

	std::array<TextMenuItem*, 5> items{&setSide[0], &setSide[1], &setSide[2], &setSide[3], &insertEject};

public:
	FDSControlView(ViewAttachParams attach):
		TableView
		{
			"FDS控制",
			attach,
			items
		}
	{
		setSide[0].setActive(0 < FCEU_FDSSides());
		setSide[1].setActive(1 < FCEU_FDSSides());
		setSide[2].setActive(2 < FCEU_FDSSides());
		setSide[3].setActive(3 < FCEU_FDSSides());
		insertEject.setActive(FCEU_FDSInserted());
	}
};

class CustomSystemActionsView : public SystemActionsView
{
private:
	TextMenuItem fdsControl
	{
		u"", attachParams(),
		[this](Input::Event e) { pushAndShow(makeView<FDSControlView>(), e); }
	};

	void refreshFDSItem()
	{
		if(!isFDS)
			return;
		if(!FCEU_FDSInserted())
			fdsControl.compile("FDS控制(无磁碟)");
		else
			fdsControl.compile(std::format("FDS控制(磁碟 {}:{})", (FCEU_FDSCurrentSide() >> 1) + 1, (FCEU_FDSCurrentSide() & 1) ? 'B' : 'A'));
	}

	TextMenuItem options
	{
		"控制台设置", attachParams(),
		[this](Input::Event e) { pushAndShow(makeView<ConsoleOptionView>(), e); }
	};

public:
	CustomSystemActionsView(ViewAttachParams attach): SystemActionsView{attach, true}
	{
		if(isFDS)
			item.emplace_back(&fdsControl);
		item.emplace_back(&options);
		loadStandardItems();
	}

	void onShow()
	{
		SystemActionsView::onShow();
		refreshFDSItem();
	}
};

class CustomSystemOptionView : public SystemOptionView, public MainAppHelper
{
	using MainAppHelper::system;

	BoolMenuItem skipFdcAccess
	{
		"快进磁碟读写", attachParams(),
		system().fastForwardDuringFdsAccess,
		[this](BoolMenuItem &item)
		{
			system().fastForwardDuringFdsAccess = item.flipBoolValue(*this);
		}
	};

public:
	CustomSystemOptionView(ViewAttachParams attach): SystemOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&skipFdcAccess);
	}
};

static std::string codeCompareToString(int compare) { return compare != -1 ? std::format("{:x}", compare) : std::string{}; }

class EditCheatView;

class EditRamCheatView: public TableView, public EmuAppHelper
{
public:
	EditRamCheatView(ViewAttachParams, Cheat&, CheatCode&, EditCheatView&);

private:
	CheatCode& code;
	EditCheatView& editCheatView;
	DualTextMenuItem addr, value, comp;
	TextMenuItem remove;
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

	void loadItems()
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

private:
	TextMenuItem addGG, addRAM;
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

private:
	TextMenuItem addGG, addRAM;
};

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

std::unique_ptr<View> EmuApp::makeCustomView(ViewAttachParams attach, ViewID id)
{
	switch(id)
	{
		case ViewID::SYSTEM_ACTIONS: return std::make_unique<CustomSystemActionsView>(attach);
		case ViewID::VIDEO_OPTIONS: return std::make_unique<CustomVideoOptionView>(attach, videoLayer);
		case ViewID::AUDIO_OPTIONS: return std::make_unique<CustomAudioOptionView>(attach, audio);
		case ViewID::SYSTEM_OPTIONS: return std::make_unique<CustomSystemOptionView>(attach);
		case ViewID::FILE_PATH_OPTIONS: return std::make_unique<CustomFilePathOptionView>(attach);
		default: return nullptr;
	}
}

std::unique_ptr<View> AppMeta::makeEditCheatsView(ViewAttachParams attach, CheatsView& view) { return std::make_unique<EditCheatsView>(attach, view); }
std::unique_ptr<View> AppMeta::makeEditCheatView(ViewAttachParams attach, Cheat& c, BaseEditCheatsView& baseView) { return std::make_unique<EditCheatView>(attach, c, baseView); }

}
