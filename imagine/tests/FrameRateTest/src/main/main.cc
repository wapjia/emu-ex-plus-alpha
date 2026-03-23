/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#include <meta.h>
import imagine;
import gui;
import cpuUtils;
import tests;
import std;
#include <imagine/base/MainInlines.hh>

namespace frameRateTest
{

using namespace IG;
using namespace cpuUtils;

constexpr SystemLogger log{"main"};
constexpr unsigned framesToRun = 60*60;

struct WindowData
{
	WindowData(ViewAttachParams attachParams):picker{attachParams} {}

	Gfx::Mat4 projM;
	WRect testRect{};
	TestPicker picker;
	std::unique_ptr<TestFramework> activeTest{};
};

static WindowData &windowData(const Window &win)
{
	return *win.appData<WindowData>();
}

class FrameRateTestApplication final: public Application
{
public:
	FrameRateTestApplication(ApplicationInitParams initParams, ApplicationContext& ctx):
		Application{initParams},
		fontManager{ctx},
		renderer{ctx}
	{
		WindowConfig winConf{ .title = ApplicationMeta::name };

		ctx.makeWindow(winConf,
			[this](ApplicationContext ctx, Window &win)
			{
				renderer.initMainTask(&win);
				Gfx::GlyphTextureSet defaultFace{renderer, fontManager.makeSystem(), win.heightScaledMMInPixels(2.5)};
				defaultFace.precacheAlphaNum(renderer);
				defaultFace.precache(renderer, ":.%()");
				viewManager.defaultFace = std::move(defaultFace);
				auto &winData = win.makeAppData<WindowData>(ViewAttachParams{viewManager, win, renderer.task()});
				std::vector<TestDesc> testDescs;
				testDescs.emplace_back(TestID::Clear, "Clear");
				WSize pixmapSize{256, 256};
				for(auto desc: renderer.textureBufferModes())
				{
					testDescs.emplace_back(TestID::Draw, std::format("Draw RGB565 {}x{} ({})", pixmapSize.x, pixmapSize.y, desc.name),
						pixmapSize, desc.mode);
					testDescs.emplace_back(TestID::Write, std::format("Write RGB565 {}x{} ({})", pixmapSize.x, pixmapSize.y, desc.name),
						pixmapSize, desc.mode);
				}
				auto &picker = winData.picker;
				picker.setTests<FrameRateTestApplication>(testDescs);
				setPickerHandlers(win);

				ctx.addOnResume(
					[&win](ApplicationContext, [[maybe_unused]] bool focused)
					{
						windowData(win).picker.prepareDraw();
						return true;
					});

				ctx.addOnExit(
					[this, &win](ApplicationContext, [[maybe_unused]] bool backgrounded)
					{
						if(backgrounded)
						{
							if(windowData(win).activeTest)
							{
								finishTest(win, SteadyClock::now());
							}
							viewManager.defaultFace.freeCaches();
						}
						return true;
					});

				win.show();
			});

		#ifdef __ANDROID__
		bool manageCPUFreq = false;
		if(manageCPUFreq)
		{
			cpuFreq.emplace();
			if(!(*cpuFreq))
			{
				cpuFreq.reset();
			}
		}
		#endif
	}

	TestFramework *startTest(Window& win, const TestParams& t)
	{
		auto ctx = win.appContext();
		#ifdef __ANDROID__
		if(cpuFreq)
			cpuFreq->setLowLatency();
		ctx.setSustainedPerformanceMode(true);
		#endif
		ViewAttachParams attach{viewManager, win, renderer.task()};
		auto &activeTest = windowData(win).activeTest;
		activeTest = [&] -> std::unique_ptr<TestFramework>
		{
			switch(t.test)
			{
				case TestID::Clear: return std::make_unique<ClearTest>(attach);
				case TestID::Draw: return std::make_unique<DrawTest>(ctx, attach, t.pixmapSize, t.bufferMode);
				case TestID::Write: return std::make_unique<WriteTest>(ctx, attach, t.pixmapSize, t.bufferMode);
			}
			unreachable();
		}();
		ctx.setIdleDisplayPowerSave(false);
		initCPUFreqStatus();
		initCPULoadStatus();
		placeElements(win);
		setActiveTestHandlers(win);
		return activeTest.get();
	}

private:
	Data::FontManager fontManager;
	Gfx::Renderer renderer;
	ViewManager viewManager;
	#ifdef __ANDROID__
	std::optional<RootCpufreqParamSetter> cpuFreq{};
	#endif

	void setActiveTestHandlers(Window& win)
	{
		win.addOnFrame([this, &win](FrameParams params)
		{
			auto atOnFrame = SteadyClock::now();
			auto &activeTest = windowData(win).activeTest;
			if(!activeTest)
			{
				return false;
			}
			if(activeTest->started)
			{
				activeTest->frameUpdate(renderer.task(), win, params);
			}
			else
			{
				activeTest->started = true;
			}
			activeTest->presentTime = params.presentTime(1);
			activeTest->lastFramePresentTime.time = params.time;
			activeTest->lastFramePresentTime.atOnFrame = atOnFrame;
			if(activeTest->frames == framesToRun || activeTest->shouldEndTest)
			{
				finishTest(win, params.time);
				return false;
			}
			else
			{
				win.setNeedsDraw(true);
				return true;
			}
		});
		win.onEvent = [this, &task = renderer.task()](Window& win, const WindowEvent& winEvent)
		{
			return winEvent.visit(overloaded
			{
				[&](const WindowSurfaceChangeEvent& e)
				{
					updateWindowSurface(win, e.change);
					return true;
				},
				[&](const DrawEvent& e)
				{
					auto xIndent = viewManager.tableXIndentPx;
					return task.draw(win, e.params, {}, [xIndent](Window &win, Gfx::RendererCommands &cmds)
					{
						auto &winData = windowData(win);
						auto &activeTest = winData.activeTest;
						auto rect = winData.testRect;
						cmds.basicEffect().setModelViewProjection(cmds, Gfx::Mat4::ident(), winData.projM);
						activeTest->draw(cmds, cmds.renderer().makeClipRect(win, rect), xIndent);
						activeTest->lastFramePresentTime.atWinPresent = SteadyClock::now();
						cmds.present(activeTest->presentTime);
					});
				},
				[&](const Input::Event& e)
				{
					auto &activeTest = windowData(win).activeTest;
					return e.visit(overloaded
					{
						[&](const Input::MotionEvent& motionEv)
						{
							if(motionEv.pushed() && Config::envIsIOS)
							{
								log.info("canceled activeTest from pointer input");
								activeTest->shouldEndTest = true;
								return true;
							}
							return false;
						},
						[&](const Input::KeyEvent& keyEv)
						{
							if(keyEv.pushed(Input::DefaultKey::CANCEL))
							{
								log.info("canceled activeTest from key input");
								activeTest->shouldEndTest = true;
								return true;
							}
							else if(keyEv.pushed(Input::Keycode::D))
							{
								log.info("posting extra draw");
								win.postDraw();
								return true;
							}
							return false;
						}
					});
				},
				[](auto&){ return false; }
			});
		};
	}

	void setPickerHandlers(Window& win)
	{
		win.onEvent = [this, &task = renderer.task()](Window& win, const WindowEvent& winEvent)
		{
			return winEvent.visit(overloaded
			{
				[&](const WindowSurfaceChangeEvent& e)
				{
					updateWindowSurface(win, e.change);
					return true;
				},
				[&](const DrawEvent& e)
				{
					return task.draw(win, e.params, {}, [](Window &win, Gfx::RendererCommands &cmds)
					{
						cmds.clear();
						auto &winData = windowData(win);
						auto &picker = winData.picker;
						cmds.basicEffect().setModelViewProjection(cmds, Gfx::Mat4::ident(), winData.projM);
						picker.draw(cmds);
						cmds.setClipTest(false);
						cmds.present();
					});
				},
				[&](const Input::Event& e)
				{
					if(e.keyEvent() && e.keyEvent()->pushed(Input::DefaultKey::CANCEL) && !e.keyEvent()->repeated())
					{
						win.appContext().exit();
						return true;
					}
					return windowData(win).picker.inputEvent(e);
				},
				[](auto&){ return false; }
			});
		};
	}

	void placeElements(const Window& win)
	{
		auto &winData = windowData(win);
		auto &picker = winData.picker;
		auto &activeTest = winData.activeTest;
		viewManager.setTableXIndentToDefault(win);
		if(!activeTest)
		{
			picker.setViewRect(win.contentBounds());
			picker.place();
		}
		else
		{
			activeTest->place(win.contentBounds(), winData.testRect);
		}
	}

	void finishTest(Window& win, SteadyClockTimePoint frameTime)
	{
		auto app = win.appContext();
		auto &activeTest = windowData(win).activeTest;
		if(activeTest)
		{
			activeTest->finish(frameTime);
		}
		renderer.mainTask.awaitPending();
		activeTest.reset();
		deinitCPUFreqStatus();
		deinitCPULoadStatus();
		app.setIdleDisplayPowerSave(true);
		#ifdef __ANDROID__
		if(cpuFreq)
			cpuFreq->setDefaults();
		app.setSustainedPerformanceMode(false);
		#endif
		placeElements(win);
		setPickerHandlers(win);
		win.postDraw();
	}

	void updateWindowSurface(Window &win, Window::SurfaceChange change)
	{
		if(change.resized())
		{
			auto viewport = win.viewport(win.contentBounds());
			renderer.task().setDefaultViewport(win, viewport);
			auto &winData = windowData(win);
			winData.projM = Gfx::Mat4::makePerspectiveFovRH(std::numbers::pi / 4.0, viewport.realAspectRatio(), 0.1, 100.)
				.projectionPlane(viewport, .5f, renderer.projectionRollAngle(win));
			winData.testRect = viewport.relRectBestFit({}, 4./3., C2DO, C2DO);
			placeElements(win);
		}
		renderer.task().updateDrawableForSurfaceChange(win, change);
	}
};

}

namespace IG
{

const char *const ApplicationMeta::name{CONFIG_APP_NAME};

void ApplicationMeta::onInit(ApplicationInitParams initParams, ApplicationContext& ctx)
{
	ctx.initApplication<frameRateTest::FrameRateTestApplication>(initParams);
}

}
