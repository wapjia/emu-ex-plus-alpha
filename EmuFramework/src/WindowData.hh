#pragma once

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

#include <emuframework/EmuViewController.hh>
#ifndef IG_USE_MODULE_IMAGINE
#include <imagine/gfx/Mat4.hh>
#endif

namespace EmuEx
{

using namespace IG;

struct WindowData
{
	Gfx::Mat4 projM;
	WRect windowRect{};
	WRect contentRect{};
	bool hasEmuView{true};
	bool hasPopup{true};
	bool focused{};

	auto windowBounds() const { return windowRect; }
	auto contentBounds() const { return contentRect; }
	void updateWindowViewport(const Window&, Viewport, const Gfx::Renderer &);

	void applyViewRect(auto &view)
	{
		view.setViewRect(contentBounds(), windowBounds());
	}
};

struct MainWindowData : public WindowData
{
	EmuViewController viewController;

	MainWindowData(ViewAttachParams attach, VController& vCtrl, EmuVideoLayer& layer, EmuSystem& system):
		viewController{attach, vCtrl, layer, system} {}
};

inline auto &windowData(const Window& win)
{
	auto data = win.appData<WindowData>();
	return *data;
}

inline auto &mainWindowData(const Window& win)
{
	auto data = win.appData<MainWindowData>();
	return *data;
}

}
