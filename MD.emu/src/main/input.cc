/*  This file is part of MD.emu.

	MD.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	MD.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with MD.emu.  If not, see <http://www.gnu.org/licenses/> */

module;
#include "genplus-config.h"
#include "input.h"
#include "system.h"
#include "loadrom.h"
#include "md_cart.h"
#include "io_ctrl.h"

module system;

namespace EmuEx
{

constexpr std::array m3MissingCodes{KeyCode(MdKey::Mode), KeyCode(MdKey::A), KeyCode(MdKey::X), KeyCode(MdKey::Y), KeyCode(MdKey::Z)};
constexpr std::array md6BtnExtraCodes{KeyCode(MdKey::Mode), KeyCode(MdKey::X), KeyCode(MdKey::Y), KeyCode(MdKey::Z)};

void MdSystem::handleInputAction(EmuApp *, InputAction a)
{
	auto player = a.flags.deviceId;
	auto key = MdKey(a.code);
	if((input.system[1] == SYSTEM_MENACER && (key == MdKey::B || key == MdKey::C)) ||
		(input.system[1] == SYSTEM_JUSTIFIER && (key == MdKey::Start)))
	{
		player = 1;
	}
	uint16 &padData = input.pad[playerIdxMap[player]];
	padData = setOrClearBits(padData, bit(a.code - 1), a.isPushed());
}

static void updateGunPos(WindowRect gameRect, const Input::MotionEvent &e, int idx)
{
	if(gameRect.overlaps(e.pos()))
	{
		int xRel = e.pos().x - gameRect.x, yRel = e.pos().y - gameRect.y;
		input.analog[idx][0] = remap(xRel, 0, gameRect.xSize(), 0, bitmap.viewport.w);
		input.analog[idx][1] = remap(yRel, 0, gameRect.ySize(), 0, bitmap.viewport.h);
	}
	else
	{
		// offscreen
		input.analog[idx][0] = input.analog[idx][1] = -1000;
	}
}

bool MdSystem::onPointerInputStart(const Input::MotionEvent &e, Input::DragTrackerState, WindowRect gameRect)
{
	if(input.dev[gunDevIdx] != DEVICE_LIGHTGUN)
		return false;
	updateGunPos(gameRect, e, gunDevIdx);
	input.pad[gunDevIdx] |= INPUT_A;
	log.info("gun pushed @ {},{}, on MD {},{}", e.pos().x, e.pos().y, input.analog[gunDevIdx][0], input.analog[gunDevIdx][1]);
	return true;
}

bool MdSystem::onPointerInputUpdate(const Input::MotionEvent &e, Input::DragTrackerState dragState,
	Input::DragTrackerState prevDragState, WindowRect gameRect)
{
	if(input.dev[gunDevIdx] != DEVICE_LIGHTGUN)
		return false;
	updateGunPos(gameRect, e, gunDevIdx);
	return true;
}

bool MdSystem::onPointerInputEnd(const Input::MotionEvent &e, Input::DragTrackerState, WindowRect)
{
	if(input.dev[gunDevIdx] != DEVICE_LIGHTGUN)
		return false;
	input.pad[gunDevIdx] = clearBits(input.pad[gunDevIdx], (uint16)INPUT_A);
	return true;
}

void MdSystem::clearInputBuffers()
{
	fill(input.pad);
	for(auto &analog : input.analog)
	{
		fill(analog);
	}
}

const char *mdInputSystemToStr(uint8 system)
{
	switch(system)
	{
		case NO_SYSTEM: return "unconnected";
		case SYSTEM_MD_GAMEPAD: return "gamepad";
		case SYSTEM_MS_GAMEPAD: return "sms gamepad";
		case SYSTEM_MOUSE: return "mouse";
		case SYSTEM_MENACER: return "menacer";
		case SYSTEM_JUSTIFIER: return "justifier";
		case SYSTEM_TEAMPLAYER: return "team-player";
		case SYSTEM_LIGHTPHASER: return "light-phaser";
		default : return "unknown";
	}
}

static bool inputPortWasAutoSetByGame(unsigned port)
{
	return old_system[port] != -1;
}

void MdSystem::setupSmsInput(EmuApp &app)
{
	// first port may be set in rom loading code
	if(!input.system[0])
		input.system[0] = SYSTEM_MS_GAMEPAD;
	input.system[1] = SYSTEM_MS_GAMEPAD;
	io_init();
	app.setDisabledInputKeys(m3MissingCodes);
	for(auto i: iotaCount(2))
	{
		log.info("attached {} to port {}", mdInputSystemToStr(input.system[i]), i);
	}
	gunDevIdx = 0;
	auto &vCtrl = app.defaultVController();
	if(input.dev[0] == DEVICE_LIGHTGUN && vCtrl.inputPlayer() != 1)
	{
		savedVControllerPlayer = vCtrl.inputPlayer();
		vCtrl.setInputPlayer(1);
	}
}

void MdSystem::setupMdInput(EmuApp &app)
{
	if(cart.special & HW_J_CART)
	{
		input.system[0] = input.system[1] = SYSTEM_MD_GAMEPAD;
		playerIdxMap[2] = 5;
		playerIdxMap[3] = 6;
	}
	else if(optionMultiTap)
	{
		input.system[0] = SYSTEM_TEAMPLAYER;
		input.system[1] = 0;

		playerIdxMap[1] = 1;
		playerIdxMap[2] = 2;
		playerIdxMap[3] = 3;
	}
	else
	{
		for(auto i: iotaCount(2))
		{
			if(mdInputPortDev[i] == -1) // user didn't specify device, go with auto settings
			{
				if(!inputPortWasAutoSetByGame(i))
					input.system[i] = SYSTEM_MD_GAMEPAD;
				else
				{
					log.info("input port {} set by game detection", i);
					input.system[i] = old_system[i];
				}
			}
			else
				input.system[i] = mdInputPortDev[i];
			log.info("attached {} to port {}{}", mdInputSystemToStr(input.system[i]), i, mdInputPortDev[i] == -1 ? " (auto)" : "");
		}
	}
	io_init();
	gunDevIdx = 4;
	if(option6BtnPad)
		app.unsetDisabledInputKeys();
	else
		app.setDisabledInputKeys(md6BtnExtraCodes);
}

void MdSystem::setupInput(EmuApp &app)
{
	if(!hasContent())
	{
		if(option6BtnPad)
			app.unsetDisabledInputKeys();
		else
			app.setDisabledInputKeys(md6BtnExtraCodes);
		return;
	}
	if(savedVControllerPlayer != -1)
	{
		app.defaultVController().setInputPlayer(std::exchange(savedVControllerPlayer, -1));
	}
	fill(playerIdxMap);
	playerIdxMap[0] = 0;
	playerIdxMap[1] = 4;

	unsigned mdPad = option6BtnPad ? DEVICE_PAD6B : DEVICE_PAD3B;
	for(auto i: iotaCount(4))
		config.input[i].padtype = mdPad;

	if(system_hw == SYSTEM_PBC)
	{
		setupSmsInput(app);
	}
	else
	{
		setupMdInput(app);
	}
}

}
