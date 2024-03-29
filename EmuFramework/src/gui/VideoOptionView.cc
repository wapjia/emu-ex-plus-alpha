/*  This file is part of EmuFramework.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with EmuFramework.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/VideoOptionView.hh>
#include <emuframework/EmuApp.hh>
#include <emuframework/EmuAppHelper.hh>
#include <emuframework/EmuVideoLayer.hh>
#include <emuframework/EmuVideo.hh>
#include <emuframework/VideoImageEffect.hh>
#include <emuframework/EmuViewController.hh>
#include <emuframework/EmuOptions.hh>
#include "PlaceVideoView.hh"
#include <imagine/base/Screen.hh>
#include <imagine/base/ApplicationContext.hh>
#include <imagine/gfx/Renderer.hh>
#include <imagine/gfx/RendererCommands.hh>
#include <imagine/gui/TextTableView.hh>
#include <format>
#include <imagine/logger/logger.h>

namespace EmuEx
{

constexpr SystemLogger log{"VideoOptionView"};

class DetectFrameRateView final: public View, public EmuAppHelper<DetectFrameRateView>
{
public:
	using DetectFrameRateDelegate = DelegateFunc<void (SteadyClockTime frameTime)>;
	DetectFrameRateDelegate onDetectFrameTime;
	IG::OnFrameDelegate detectFrameRate;
	SteadyClockTime totalFrameTime{};
	SteadyClockTimePoint lastFrameTimestamp{};
	Gfx::Text fpsText;
	int allTotalFrames{};
	int callbacks{};
	std::vector<SteadyClockTime> frameTimeSample{};
	bool useRenderTaskTime = false;

	DetectFrameRateView(ViewAttachParams attach): View(attach),
		fpsText{attach.rendererTask, &defaultFace()}
	{
		defaultFace().precacheAlphaNum(attach.renderer());
		defaultFace().precache(attach.renderer(), ".");
		fpsText.resetString("准备检测刷新率...");
		useRenderTaskTime = !screen()->supportsTimestamps();
		frameTimeSample.reserve(std::round(screen()->frameRate() * 2.));
	}

	~DetectFrameRateView() final
	{
		window().setIntendedFrameRate(0);
		app().setCPUNeedsLowLatency(appContext(), false);
		window().removeOnFrame(detectFrameRate);
	}

	void place() final
	{
		fpsText.compile();
	}

	bool inputEvent(const Input::Event &e) final
	{
		if(e.keyEvent() && e.keyEvent()->pushed(Input::DefaultKey::CANCEL))
		{
			log.info("aborted detection");
			dismiss();
			return true;
		}
		return false;
	}

	void draw(Gfx::RendererCommands &__restrict__ cmds) final
	{
		using namespace IG::Gfx;
		cmds.basicEffect().enableAlphaTexture(cmds);
		fpsText.draw(cmds, viewRect().center(), C2DO, ColorName::WHITE);
	}

	bool runFrameTimeDetection(SteadyClockTime timestampDiff, double slack)
	{
		const int framesToTime = frameTimeSample.capacity() * 10;
		allTotalFrames++;
		frameTimeSample.emplace_back(timestampDiff);
		if(frameTimeSample.size() == frameTimeSample.capacity())
		{
			bool stableFrameTime = true;
			SteadyClockTime frameTimeTotal{};
			{
				SteadyClockTime lastFrameTime{};
				for(auto frameTime : frameTimeSample)
				{
					frameTimeTotal += frameTime;
					if(!stableFrameTime)
						continue;
					double frameTimeDiffSecs =
						std::abs(IG::FloatSeconds(lastFrameTime - frameTime).count());
					if(lastFrameTime.count() && frameTimeDiffSecs > slack)
					{
						log.info("frame times differed by:{}", frameTimeDiffSecs);
						stableFrameTime = false;
					}
					lastFrameTime = frameTime;
				}
			}
			auto frameTimeTotalSecs = FloatSeconds(frameTimeTotal);
			auto detectedFrameTimeSecs = frameTimeTotalSecs / (double)frameTimeSample.size();
			auto detectedFrameTime = round<SteadyClockTime>(detectedFrameTimeSecs);
			{
				if(detectedFrameTime.count())
					fpsText.resetString(std::format("{:g}fps", toHz(detectedFrameTimeSecs)));
				else
					fpsText.resetString("0fps");
				fpsText.compile();
			}
			if(stableFrameTime)
			{
				log.info("found frame time:{}", detectedFrameTimeSecs);
				onDetectFrameTime(detectedFrameTime);
				dismiss();
				return false;
			}
			frameTimeSample.erase(frameTimeSample.cbegin());
			postDraw();
		}
		else
		{
			//log.info("waiting for capacity:{}/{}", frameTimeSample.size(), frameTimeSample.capacity());
		}
		if(allTotalFrames >= framesToTime)
		{
			onDetectFrameTime(SteadyClockTime{});
			dismiss();
			return false;
		}
		else
		{
			if(useRenderTaskTime)
				postDraw();
			return true;
		}
	}

	void onAddedToController(ViewController *, const Input::Event &e) final
	{
		lastFrameTimestamp = SteadyClock::now();
		detectFrameRate =
			[this](IG::FrameParams params)
			{
				const int callbacksToSkip = 10;
				callbacks++;
				if(callbacks < callbacksToSkip)
				{
					if(useRenderTaskTime)
						postDraw();
					return true;
				}
				return runFrameTimeDetection(params.timestamp - std::exchange(lastFrameTimestamp, params.timestamp), 0.00175);
			};
		window().addOnFrame(detectFrameRate);
		app().setCPUNeedsLowLatency(appContext(), true);
	}
};

static std::string makeFrameRateStr(VideoSystem vidSys, const OutputTimingManager &mgr)
{
	auto frameTimeOpt = mgr.frameTimeOption(vidSys);
	if(frameTimeOpt == OutputTimingManager::autoOption)
		return "自动";
	else if(frameTimeOpt == OutputTimingManager::originalOption)
		return "原始";
	else
		return std::format("{:g}Hz", toHz(frameTimeOpt));
}

static const char *autoWindowPixelFormatStr(IG::ApplicationContext ctx)
{
	return ctx.defaultWindowPixelFormat() == PIXEL_RGB565 ? "RGB565" : "RGBA8888";
}

constexpr uint16_t pack(Gfx::DrawableConfig c)
{
	return to_underlying(c.pixelFormat.id()) | to_underlying(c.colorSpace) << sizeof(c.colorSpace) * 8;
}

constexpr Gfx::DrawableConfig unpackDrawableConfig(uint16_t c)
{
	return {PixelFormatID(c & 0xFF), Gfx::ColorSpace(c >> sizeof(Gfx::DrawableConfig::colorSpace) * 8)};
}

VideoOptionView::VideoOptionView(ViewAttachParams attach, bool customMenu):
	TableView{"视频设置", attach, item},
	textureBufferModeItem
	{
		[&]
		{
			decltype(textureBufferModeItem) items;
			items.emplace_back("自动(设置最佳模式)", attach, [this](View &view)
			{
				app().textureBufferMode = Gfx::TextureBufferMode::DEFAULT;
				auto defaultMode = renderer().makeValidTextureBufferMode();
				emuVideo().setTextureBufferMode(system(), defaultMode);
				textureBufferMode.setSelected(MenuId{defaultMode});
				view.dismiss();
				return false;
			}, MenuItem::Config{.id = 0});
			for(auto desc: renderer().textureBufferModes())
			{
				items.emplace_back(desc.name, attach, [this](MenuItem &item)
				{
					app().textureBufferMode = Gfx::TextureBufferMode(item.id.val);
					emuVideo().setTextureBufferMode(system(), Gfx::TextureBufferMode(item.id.val));
				}, MenuItem::Config{.id = desc.mode});
			}
			return items;
		}()
	},
	textureBufferMode
	{
		"GPU复制模式", attach,
		MenuId{renderer().makeValidTextureBufferMode(app().textureBufferMode)},
		textureBufferModeItem
	},
	frameIntervalItem
	{
		{"满速(不跳帧)", attach, {.id = 0}},
		{"Full",           attach, {.id = 1}},
		{"1/2",            attach, {.id = 2}},
		{"1/3",            attach, {.id = 3}},
		{"1/4",            attach, {.id = 4}},
	},
	frameInterval
	{
		"目标帧率", attach,
		MenuId{app().frameInterval},
		frameIntervalItem,
		MultiChoiceMenuItem::Config
		{
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().frameInterval.setUnchecked(item.id); }
		},
	},
	frameRateItems
	{
		{"自动(帧数相近时使用屏幕刷新率)", attach,
			[this]
			{
				if(!app().viewController().emuWindowScreen()->frameRateIsReliable())
				{
					app().postErrorMessage("报告的刷新率可能不可靠,"
						"使用检测到的刷新率可能会更好");
				}
				onFrameTimeChange(activeVideoSystem, OutputTimingManager::autoOption);
			}, {.id = OutputTimingManager::autoOption.count()}
		},
		{"原始(使用模拟系统的速率)", attach,
			[this]
			{
				onFrameTimeChange(activeVideoSystem, OutputTimingManager::originalOption);
			}, {.id = OutputTimingManager::originalOption.count()}
		},
		{"检测屏幕刷新率并设置", attach,
			[this](const Input::Event &e)
			{
				window().setIntendedFrameRate(system().frameRate());
				auto frView = makeView<DetectFrameRateView>();
				frView->onDetectFrameTime =
					[this](SteadyClockTime frameTime)
					{
						if(frameTime.count())
						{
							if(onFrameTimeChange(activeVideoSystem, frameTime))
								dismissPrevious();
						}
						else
						{
							app().postErrorMessage("检测到的刷新率太不稳定而无法使用");
						}
					};
				pushAndShowModal(std::move(frView), e);
				return false;
			}
		},
		{"自定义刷新率", attach,
			[this](const Input::Event &e)
			{
				app().pushAndShowNewCollectValueInputView<std::pair<double, double>>(attachParams(), e,
					"输入整数或小数", "",
					[this](EmuApp &, auto val)
					{
						if(onFrameTimeChange(activeVideoSystem, fromSeconds<SteadyClockTime>(val.second / val.first)))
						{
							if(activeVideoSystem == VideoSystem::NATIVE_NTSC)
								frameRate.setSelected(defaultMenuId, *this);
							else
								frameRatePAL.setSelected(defaultMenuId, *this);
							dismissPrevious();
							return true;
						}
						else
							return false;
					});
				return false;
			}, {.id = defaultMenuId}
		},
	},
	frameRate
	{
		"刷新率", attach,
		app().outputTimingManager.frameTimeOptionAsMenuId(VideoSystem::NATIVE_NTSC),
		frameRateItems,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(makeFrameRateStr(VideoSystem::NATIVE_NTSC, app().outputTimingManager));
				return true;
			},
			.onSelect = [this](MultiChoiceMenuItem &item, View &view, const Input::Event &e)
			{
				activeVideoSystem = VideoSystem::NATIVE_NTSC;
				item.defaultOnSelect(view, e);
			},
		},
	},
	frameRatePAL
	{
		"刷新率 (PAL)", attach,
		app().outputTimingManager.frameTimeOptionAsMenuId(VideoSystem::PAL),
		frameRateItems,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(makeFrameRateStr(VideoSystem::PAL, app().outputTimingManager));
				return true;
			},
			.onSelect = [this](MultiChoiceMenuItem &item, View &view, const Input::Event &e)
			{
				activeVideoSystem = VideoSystem::PAL;
				item.defaultOnSelect(view, e);
			},
		},
	},
	frameTimeStats
	{
		"显示帧时间统计信息", attach,
		app().showFrameTimeStats,
		[this](BoolMenuItem &item) { app().showFrameTimeStats = item.flipBoolValue(*this); }
	},
	aspectRatioItem
	{
		[&]()
		{
			StaticArrayList<TextMenuItem, MAX_ASPECT_RATIO_ITEMS> aspectRatioItem;
			for(const auto &i : EmuSystem::aspectRatioInfos())
			{
				aspectRatioItem.emplace_back(i.name, attach, [this](TextMenuItem &item)
				{
					app().setVideoAspectRatio(std::bit_cast<float>(item.id));
				}, MenuItem::Config{.id = std::bit_cast<MenuId>(i.aspect.ratio<float>())});
			}
			if(EmuSystem::hasRectangularPixels)
			{
				aspectRatioItem.emplace_back("正方形像素", attach, [this]()
				{
					app().setVideoAspectRatio(-1);
				}, MenuItem::Config{.id = std::bit_cast<MenuId>(-1.f)});
			}
			aspectRatioItem.emplace_back("填充屏幕", attach, [this]()
			{
				app().setVideoAspectRatio(0);
			}, MenuItem::Config{.id = 0});
			aspectRatioItem.emplace_back("自定义数值", attach, [this](const Input::Event &e)
			{
				app().pushAndShowNewCollectValueInputView<std::pair<float, float>>(attachParams(), e,
					"输入整数或小数", "",
					[this](EmuApp &app, auto val)
					{
						float ratio = val.first / val.second;
						if(app.setVideoAspectRatio(ratio))
						{
							aspectRatio.setSelected(std::bit_cast<MenuId>(ratio), *this);
							dismissPrevious();
							return true;
						}
						else
						{
							app.postErrorMessage("输入错误");
							return false;
						}
					});
				return false;
			}, MenuItem::Config{.id = defaultMenuId});
			return aspectRatioItem;
		}()
	},
	aspectRatio
	{
		"显示比例", attach,
		std::bit_cast<MenuId>(app().videoAspectRatio()),
		aspectRatioItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(idx == aspectRatioItem.size() - 1)
				{
					t.resetString(std::format("{:g}", app().videoAspectRatio()));
					return true;
				}
				return false;
			}
		},
	},
	zoomItem
	{
		{"100%",                  attach, {.id = 100}},
		{"90%",                   attach, {.id = 90}},
		{"80%",                   attach, {.id = 80}},
		{"仅整数倍",          attach, {.id = optionImageZoomIntegerOnly}},
		{"仅整数倍(高度)", attach, {.id = optionImageZoomIntegerOnlyY}},
		{"自定义数值", attach,
			[this](const Input::Event &e)
			{
				app().pushAndShowNewCollectValueRangeInputView<int, 10, 200>(attachParams(), e, "输入10到200", "",
					[this](EmuApp &app, auto val)
					{
						app.setVideoZoom(val);
						zoom.setSelected(MenuId{val}, *this);
						dismissPrevious();
						return true;
					});
				return false;
			}, {.id = defaultMenuId}
		},
	},
	zoom
	{
		"画面缩放", attach,
		MenuId{app().imageZoom},
		zoomItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(app().imageZoom <= 200)
				{
					t.resetString(std::format("{}%", app().imageZoom.value()));
					return true;
				}
				return false;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().setVideoZoom(item.id); }
		},
	},
	viewportZoomItem
	{
		{"100%", attach, {.id = 100}},
		{"95%", attach,  {.id = 95}},
		{"90%", attach,  {.id = 90}},
		{"自定义", attach,
			[this](const Input::Event &e)
			{
				app().pushAndShowNewCollectValueRangeInputView<int, 50, 100>(attachParams(), e, "输入50到100", "",
					[this](EmuApp &app, auto val)
					{
						app.setViewportZoom(val);
						viewportZoom.setSelected(MenuId{val}, *this);
						dismissPrevious();
						return true;
					});
				return false;
			}, {.id = defaultMenuId}
		},
	},
	viewportZoom
	{
		"应用缩放", attach,
		MenuId{app().viewportZoom},
		viewportZoomItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format("{}%", app().viewportZoom.value()));
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().setViewportZoom(item.id); }
		},
	},
	contentRotationItem
	{
		{"自动",        attach, {.id = Rotation::ANY}},
		{"标准",    attach, {.id = Rotation::UP}},
		{"右旋转90°",   attach, {.id = Rotation::RIGHT}},
		{"上下翻转", attach, {.id = Rotation::DOWN}},
		{"左旋转90°",    attach, {.id = Rotation::LEFT}},
	},
	contentRotation
	{
		"画面旋转", attach,
		MenuId{app().contentRotation.value()},
		contentRotationItem,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().setContentRotation(Rotation(item.id.val)); }
		},
	},
	placeVideo
	{
		"设置画面位置", attach,
		[this](const Input::Event &e)
		{
			if(!system().hasContent())
				return;
			pushAndShowModal(makeView<PlaceVideoView>(*videoLayer, app().defaultVController()), e);
		}
	},
	imgFilter
	{
		"图像插值", attach,
		app().videoLayer.usingLinearFilter(),
		"无", "线性",
		[this](BoolMenuItem &item)
		{
			videoLayer->setLinearFilter(item.flipBoolValue(*this));
			app().viewController().postDrawToEmuWindows();
		}
	},
	imgEffectItem
	{
		{"关",         attach, {.id = ImageEffectId::DIRECT}},
		{"hq2x",        attach, {.id = ImageEffectId::HQ2X}},
		{"缩放2x",     attach, {.id = ImageEffectId::SCALE2X}},
		{"预缩放2x", attach, {.id = ImageEffectId::PRESCALE2X}},
		{"预缩放3x", attach, {.id = ImageEffectId::PRESCALE3X}},
		{"预缩放4x", attach, {.id = ImageEffectId::PRESCALE4X}},
	},
	imgEffect
	{
		"图像效果", attach,
		MenuId{app().videoLayer.effectId()},
		imgEffectItem,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				videoLayer->setEffect(system(), ImageEffectId(item.id.val), app().videoEffectPixelFormat());
				app().viewController().postDrawToEmuWindows();
			}
		},
	},
	overlayEffectItem
	{
		{"关",            attach, {.id = 0}},
		{"扫描线",      attach, {.id = ImageOverlayId::SCANLINES}},
		{"扫描线2x",   attach, {.id = ImageOverlayId::SCANLINES_2}},
		{"LCD Grid",       attach, {.id = ImageOverlayId::LCD}},
		{"CRT Mask",       attach, {.id = ImageOverlayId::CRT_MASK}},
		{"CRT Mask .5x",   attach, {.id = ImageOverlayId::CRT_MASK_2}},
		{"CRT Grille",     attach, {.id = ImageOverlayId::CRT_GRILLE}},
		{"CRT Grille .5x", attach, {.id = ImageOverlayId::CRT_GRILLE_2}}
	},
	overlayEffect
	{
		"叠加效果", attach,
		MenuId{app().videoLayer.overlayEffectId()},
		overlayEffectItem,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				videoLayer->setOverlay(ImageOverlayId(item.id.val));
				app().viewController().postDrawToEmuWindows();
			}
		},
	},
	overlayEffectLevelItem
	{
		{"100%", attach, {.id = 100}},
		{"75%",  attach, {.id = 75}},
		{"50%",  attach, {.id = 50}},
		{"25%",  attach, {.id = 25}},
		{"自定义", attach,
			[this](const Input::Event &e)
			{
				app().pushAndShowNewCollectValueRangeInputView<int, 0, 100>(attachParams(), e, "输入0到100", "",
					[this](EmuApp &app, auto val)
					{
						videoLayer->setOverlayIntensity(val / 100.f);
						app.viewController().postDrawToEmuWindows();
						overlayEffectLevel.setSelected(MenuId{val}, *this);
						dismissPrevious();
						return true;
					});
				return false;
			}, {.id = defaultMenuId}
		},
	},
	overlayEffectLevel
	{
		"叠加效果级别", attach,
		MenuId{app().videoLayer.overlayIntensity() * 100.f},
		overlayEffectLevelItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format("{}%", int(videoLayer->overlayIntensity() * 100.f)));
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				videoLayer->setOverlayIntensity(item.id / 100.f);
				app().viewController().postDrawToEmuWindows();
			}
		},
	},
	imgEffectPixelFormatItem
	{
		{"自动(匹配显示格式)", attach, {.id = PIXEL_NONE}},
		{"RGBA8888",                    attach, {.id = PIXEL_RGBA8888}},
		{"RGB565",                      attach, {.id = PIXEL_RGB565}},
	},
	imgEffectPixelFormat
	{
		"效果颜色格式", attach,
		MenuId{app().imageEffectPixelFormat},
		imgEffectPixelFormatItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(idx == 0)
				{
					t.resetString(app().videoEffectPixelFormat().name());
					return true;
				}
				else
					return false;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				app().imageEffectPixelFormat = PixelFormatID(item.id.val);
				videoLayer->setEffectFormat(app().videoEffectPixelFormat());
				app().viewController().postDrawToEmuWindows();
			}
		},
	},
	windowPixelFormatItem
	{
		[&]
		{
			decltype(windowPixelFormatItem) items;
			auto setWindowDrawableConfigDel = [this](TextMenuItem &item)
			{
				auto conf = unpackDrawableConfig(item.id);
				if(!app().setWindowDrawableConfig(conf))
				{
					app().postMessage("重启APP后设置生效");
					return;
				}
				renderPixelFormat.updateDisplayString();
				imgEffectPixelFormat.updateDisplayString();
			};
			items.emplace_back("自动", attach, setWindowDrawableConfigDel, MenuItem::Config{.id = 0});
			for(auto desc: renderer().supportedDrawableConfigs())
			{
				items.emplace_back(desc.name, attach, setWindowDrawableConfigDel, MenuItem::Config{.id = pack(desc.config)});
			}
			return items;
		}()
	},
	windowPixelFormat
	{
		"显示颜色格式", attach,
		MenuId{pack(app().windowDrawableConfig())},
		windowPixelFormatItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(idx == 0)
				{
					t.resetString(autoWindowPixelFormatStr(appContext()));
					return true;
				}
				else
					return false;
			}
		},
	},
	secondDisplay
	{
		"2nd Window (for testing only)", attach,
		false,
		[this](BoolMenuItem &item)
		{
			app().setEmuViewOnExtraWindow(item.flipBoolValue(*this), appContext().mainScreen());
		}
	},
	showOnSecondScreen
	{
		"外接屏幕", attach,
		app().showOnSecondScreen,
		"系统管理", "游戏画面",
		[this](BoolMenuItem &item)
		{
			app().showOnSecondScreen = item.flipBoolValue(*this);
			if(appContext().screens().size() > 1)
				app().setEmuViewOnExtraWindow(app().showOnSecondScreen, *appContext().screens()[1]);
		}
	},
	frameClockItems
	{
		{"自动",                                  attach, MenuItem::Config{.id = FrameTimeSource::Unset}},
		{"屏幕(延迟更低，功耗更小)",     attach, MenuItem::Config{.id = FrameTimeSource::Screen}},
		{"渲染器(可能缓存多个帧)", attach, MenuItem::Config{.id = FrameTimeSource::Renderer}},
	},
	frameClock
	{
		"图像缓冲区", attach,
		MenuId{FrameTimeSource(app().frameTimeSource)},
		frameClockItems,
		MultiChoiceMenuItem::Config
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(wise_enum::to_string(app().effectiveFrameTimeSource()));
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				app().frameTimeSource = FrameTimeSource(item.id.val);
				app().video.resetImage(); // texture can switch between single/double buffered
			}
		},
	},
	presentModeItems
	{
		{"自动",                                                 attach, MenuItem::Config{.id = Gfx::PresentMode::Auto}},
		{"立即(延迟较低，但可能会丢帧)", attach, MenuItem::Config{.id = Gfx::PresentMode::Immediate}},
		{"队列(帧率更稳)",                 attach, MenuItem::Config{.id = Gfx::PresentMode::FIFO}},
	},
	presentMode
	{
		"呈现模式", attach,
		MenuId{Gfx::PresentMode(app().presentMode)},
		presentModeItems,
		MultiChoiceMenuItem::Config
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(renderer().evalPresentMode(app().emuWindow(), app().presentMode) == Gfx::PresentMode::FIFO ? "Queued" : "Immediate");
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				app().presentMode = Gfx::PresentMode(item.id.val);
			}
		},
	},
	renderPixelFormatItem
	{
		{"自动(根据需要匹配渲染格式)", attach, {.id = PIXEL_NONE}},
		{"RGBA8888",                    attach, {.id = PIXEL_RGBA8888}},
		{"RGB565",                      attach, {.id = PIXEL_RGB565}},
	},
	renderPixelFormat
	{
		"渲染颜色格式", attach,
		MenuId{app().renderPixelFormat().id()},
		renderPixelFormatItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(idx == 0)
				{
					t.resetString(emuVideo().internalRenderPixelFormat().name());
					return true;
				}
				return false;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().setRenderPixelFormat(PixelFormatID(item.id.val)); }
		},
	},
	screenFrameRateItems
	{
		[&]
		{
			std::vector<TextMenuItem> items;
			auto setRateDel = [this](TextMenuItem &item) { app().overrideScreenFrameRate = std::bit_cast<FrameRate>(item.id); };
			items.emplace_back("关", attach, setRateDel, MenuItem::Config{.id = 0});
			for(auto rate : app().emuScreen().supportedFrameRates())
				items.emplace_back(std::format("{:g}Hz", rate), attach, setRateDel, MenuItem::Config{.id = std::bit_cast<MenuId>(rate)});
			return items;
		}()
	},
	screenFrameRate
	{
		"覆盖屏幕帧率", attach,
		std::bit_cast<MenuId>(FrameRate(app().overrideScreenFrameRate)),
		screenFrameRateItems
	},
	presentationTimeItems
	{
		{"全面模式(适用于所有帧率目标)",         attach, MenuItem::Config{.id = PresentationTimeMode::full}},
		{"基础模式(仅适用于较低的帧率目标)", attach, MenuItem::Config{.id = PresentationTimeMode::basic}},
		{"Off",                                            attach, MenuItem::Config{.id = PresentationTimeMode::off}},
	},
	presentationTime
	{
		"精确帧同步", attach,
		MenuId{PresentationTimeMode(app().presentationTimeMode)},
		presentationTimeItems,
		MultiChoiceMenuItem::Config
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(app().presentationTimeMode == PresentationTimeMode::off)
					return false;
				t.resetString(app().presentationTimeMode == PresentationTimeMode::full ? "Full" : "Basic");
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				app().presentationTimeMode = PresentationTimeMode(item.id.val);
			}
		},
	},
	blankFrameInsertion
	{
		"允许插入空白帧", attach,
		app().allowBlankFrameInsertion,
		[this](BoolMenuItem &item) { app().allowBlankFrameInsertion = item.flipBoolValue(*this); }
	},
	brightnessItem
	{
		{
			"默认", attach, [this](View &v)
			{
				app().setVideoBrightness(1.f, ImageChannel::All);
				setAllColorLevelsSelected(MenuId{100});
				v.dismiss();
			}
		},
		{"自定义", attach, setVideoBrightnessCustomDel(ImageChannel::All)},
	},
	redItem
	{
		{"默认", attach, [this](){ app().setVideoBrightness(1.f, ImageChannel::Red); }, {.id = 100}},
		{"自定义数值", attach, setVideoBrightnessCustomDel(ImageChannel::Red), {.id = defaultMenuId}},
	},
	greenItem
	{
		{"默认", attach, [this](){ app().setVideoBrightness(1.f, ImageChannel::Green); }, {.id = 100}},
		{"自定义数值", attach, setVideoBrightnessCustomDel(ImageChannel::Green), {.id = defaultMenuId}},
	},
	blueItem
	{
		{"默认", attach, [this](){ app().setVideoBrightness(1.f, ImageChannel::Blue); }, {.id = 100}},
		{"自定义数值", attach, setVideoBrightnessCustomDel(ImageChannel::Blue), {.id = defaultMenuId}},
	},
	brightness
	{
		"设置所有级别", attach,
		[this](const Input::Event &e)
		{
			pushAndShow(makeViewWithName<TableView>("所有级别", brightnessItem), e);
		}
	},
	red
	{
		"红", attach,
		MenuId{app().videoBrightnessAsInt(ImageChannel::Red)},
		redItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format("{}%", app().videoBrightnessAsInt(ImageChannel::Red)));
				return true;
			}
		},
	},
	green
	{
		"绿", attach,
		MenuId{app().videoBrightnessAsInt(ImageChannel::Green)},
		greenItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format("{}%", app().videoBrightnessAsInt(ImageChannel::Green)));
				return true;
			}
		},
	},
	blue
	{
		"蓝", attach,
		MenuId{app().videoBrightnessAsInt(ImageChannel::Blue)},
		blueItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format("{}%", app().videoBrightnessAsInt(ImageChannel::Blue)));
				return true;
			}
		},
	},
	visualsHeading{"视觉效果", attach},
	screenShapeHeading{"屏幕形状", attach},
	colorLevelsHeading{"颜色级别", attach},
	advancedHeading{"高级", attach},
	systemSpecificHeading{"系统特定", attach}
{
	if(!customMenu)
	{
		loadStockItems();
	}
}

void VideoOptionView::place()
{
	aspectRatio.setSelected(std::bit_cast<MenuId>(app().videoAspectRatio()), *this);
	TableView::place();
}


void VideoOptionView::loadStockItems()
{
	item.emplace_back(&frameInterval);
	item.emplace_back(&frameRate);
	if(EmuSystem::hasPALVideoSystem)
	{
		item.emplace_back(&frameRatePAL);
	}
	if(used(frameTimeStats))
		item.emplace_back(&frameTimeStats);
	item.emplace_back(&visualsHeading);
	item.emplace_back(&imgFilter);
	item.emplace_back(&imgEffect);
	item.emplace_back(&overlayEffect);
	item.emplace_back(&overlayEffectLevel);
	item.emplace_back(&screenShapeHeading);
	item.emplace_back(&zoom);
	item.emplace_back(&viewportZoom);
	item.emplace_back(&aspectRatio);
	item.emplace_back(&contentRotation);
	placeVideo.setActive(system().hasContent());
	item.emplace_back(&placeVideo);
	item.emplace_back(&colorLevelsHeading);
	item.emplace_back(&brightness);
	item.emplace_back(&red);
	item.emplace_back(&green);
	item.emplace_back(&blue);
	item.emplace_back(&advancedHeading);
	item.emplace_back(&textureBufferMode);
	if(windowPixelFormatItem.size() > 2)
	{
		item.emplace_back(&windowPixelFormat);
	}
	if(EmuSystem::canRenderRGBA8888)
		item.emplace_back(&renderPixelFormat);
	item.emplace_back(&imgEffectPixelFormat);
	item.emplace_back(&frameClock);
	if(used(presentMode))
		item.emplace_back(&presentMode);
	if(used(presentationTime) && renderer().supportsPresentationTime())
		item.emplace_back(&presentationTime);
	item.emplace_back(&blankFrameInsertion);
	if(used(screenFrameRate) && app().emuScreen().supportedFrameRates().size() > 1)
		item.emplace_back(&screenFrameRate);
	if(used(secondDisplay))
		item.emplace_back(&secondDisplay);
	if(used(showOnSecondScreen) && app().supportsShowOnSecondScreen(appContext()))
		item.emplace_back(&showOnSecondScreen);
}

void VideoOptionView::setEmuVideoLayer(EmuVideoLayer &videoLayer_)
{
	videoLayer = &videoLayer_;
}

bool VideoOptionView::onFrameTimeChange(VideoSystem vidSys, SteadyClockTime time)
{
	if(!app().outputTimingManager.setFrameTimeOption(vidSys, time))
	{
		app().postMessage(4, true, std::format("{:g}Hz 不在有效范围内", toHz(time)));
		return false;
	}
	return true;
}

TextMenuItem::SelectDelegate VideoOptionView::setVideoBrightnessCustomDel(ImageChannel ch)
{
	return [=, this](const Input::Event &e)
	{
		app().pushAndShowNewCollectValueRangeInputView<int, 0, 200>(attachParams(), e, "输入0到200", "",
			[=, this](EmuApp &app, auto val)
			{
				app.setVideoBrightness(val / 100.f, ch);
				if(ch == ImageChannel::All)
					setAllColorLevelsSelected(MenuId{val});
				else
					[&]() -> MultiChoiceMenuItem&
					{
						switch(ch)
						{
							case ImageChannel::All: break;
							case ImageChannel::Red: return red;
							case ImageChannel::Green: return green;
							case ImageChannel::Blue: return blue;
						}
						bug_unreachable("invalid ImageChannel");
					}().setSelected(MenuId{val}, *this);
				dismissPrevious();
				return true;
			});
		return false;
	};
}

void VideoOptionView::setAllColorLevelsSelected(MenuId val)
{
	red.setSelected(val, *this);
	green.setSelected(val, *this);
	blue.setSelected(val, *this);
}

EmuVideo &VideoOptionView::emuVideo() const
{
	return videoLayer->emuVideo();
}

}
