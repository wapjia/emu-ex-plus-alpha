/*  This file is part of Saturn.emu.

	Saturn.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Saturn.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Saturn.emu.  If not, see <http://www.gnu.org/licenses/> */

module;
#include <mednafen/mednafen.h>
#include <mednafen/state.h>
#include <mednafen/video/surface.h>
#include <ss/ss.h>
#include <ss/smpc.h>
#include <ss/smpc_iodevice.h>
#include <ss/input/gamepad.h>
#include <ss/input/3dpad.h>
#include <ss/input/mouse.h>
#include <ss/input/wheel.h>
#include <ss/input/mission.h>
#include <ss/input/gun.h>
#include <ss/input/keyboard.h>
#include <ss/input/jpkeyboard.h>

module system;

namespace EmuEx
{

struct GunData
{
	int16 x, y;
	uint8_t trigger:1, start:1, offscreen:1;
};

inline GunData& asGunData(uint64_t& inputData) { return reinterpret_cast<GunData&>(inputData); }

void SaturnSystem::handleInputAction(EmuApp *, InputAction a)
{
	auto player = a.flags.deviceId;
	assume(player < AppMeta::maxPlayers);
	if(inputConfig.devs[player] == InputDeviceType::gun)
	{
		if(SaturnKey(a.code) == SaturnKey::Start)
		{
			asGunData(inputBuff[player]).start = a.state == Input::Action::PUSHED;
		}
	}
	else
	{
		auto oppositeKey = [](SaturnKey key)
		{
			switch(key)
			{
				case SaturnKey::Up:    return SaturnKey::Down;
				case SaturnKey::Right: return SaturnKey::Left;
				case SaturnKey::Down:  return SaturnKey::Up;
				case SaturnKey::Left:  return SaturnKey::Right;
				default: return SaturnKey{};
			}
		}(SaturnKey(a.code));
		if(a.state == Input::Action::PUSHED && int(oppositeKey)) // un-set opposite directions, otherwise many games report a gamepad disconnect
		{
			inputBuff[player] = clearBits(inputBuff[player], bit(to_underlying(oppositeKey) - 1));
		}
		inputBuff[player] = setOrClearBits(inputBuff[player], bit(a.code - 1), a.state == Input::Action::PUSHED);
	}
}

void SaturnSystem::clearInputBuffers()
{
	inputBuff = {};
}

static const char *toString(InputDeviceType dev)
{
	using enum InputDeviceType;
	switch(dev)
	{
		case none: return "none";
		case gamepad: return "gamepad";
		case multipad: return "3dpad";
		case mouse: return "mouse";
		case wheel: return "wheel";
		case mission: return "mission";
		case dmission: return "dmission";
		case gun: return "gun";
		case keyboard: return "keyboard";
		case jpkeyboard: return "jpkeyboard";
	}
	std::unreachable();
}

void SaturnSystem::applyInputConfig(InputConfig config, EmuApp &app)
{
	if(!CDInterfaces.size())
		return; // no content
	using namespace MDFN_IEN_SS;
	SMPC_SetMultitap(0, config.multitaps[0]);
	SMPC_SetMultitap(1, config.multitaps[1]);
	for(auto [idx, dev] : enumerate(config.devs))
	{
		mdfnGameInfo.SetInput(idx, toString(dev), reinterpret_cast<uint8*>(&inputBuff[idx]));
	}
	if(inputConfig.devs[0] == InputDeviceType::gun)
	{
		constexpr std::array gunDisabledKeys{
			KeyCode(SaturnKey::A), KeyCode(SaturnKey::B),	KeyCode(SaturnKey::C),
			KeyCode(SaturnKey::X), KeyCode(SaturnKey::Y), KeyCode(SaturnKey::Z),
			KeyCode(SaturnKey::L), KeyCode(SaturnKey::R)};
		app.setDisabledInputKeys(gunDisabledKeys);
		app.defaultVController().setGamepadDPadIsEnabled(false);
	}
	else
	{
		app.unsetDisabledInputKeys();
		app.defaultVController().setGamepadDPadIsEnabled(true);
	}
	currStateSize = stateSizeMDFN(); // changing input devices affects state size
}

void SaturnSystem::setInputConfig(InputConfig config, EmuApp &app)
{
	inputConfig = config;
	applyInputConfig(config, app);
}

bool SaturnSystem::onPointerInputStart(const Input::MotionEvent &e, Input::DragTrackerState, WRect gameRect)
{
	if(inputConfig.devs[0] != InputDeviceType::gun)
		return false;
	auto &gunData = asGunData(inputBuff[0]);
	if(gameRect.overlaps(e.pos()) && espec.DisplayRect.h)
	{
		auto xRel = e.pos().x - gameRect.x, yRel = e.pos().y - gameRect.y;
		auto xGun = remap(xRel, 0, gameRect.xSize(), 0.f, mdfnGameInfo.mouse_scale_x) + mdfnGameInfo.mouse_offs_x;
		auto yGun = remap(yRel, 0, gameRect.ySize(), 0.f, mdfnGameInfo.mouse_scale_y) + mdfnGameInfo.mouse_offs_y;
		//log.debug("gun pushed @ {},{}, mapped to {},{}", e.pos().x, e.pos().y, xGun, yGun);
		gunData.x = xGun;
		gunData.y = yGun;
		gunData.trigger = 1;
	}
	else
	{
		//log.debug("gun offscreen");
		gunData.offscreen = 1;
	}
	return true;
}

bool SaturnSystem::onPointerInputEnd(const Input::MotionEvent &, Input::DragTrackerState, WRect)
{
	if(inputConfig.devs[0] != InputDeviceType::gun)
		return false;
	auto &gunData = asGunData(inputBuff[0]);
	gunData.trigger = 0;
	gunData.offscreen = 0;
	return true;
}

}
