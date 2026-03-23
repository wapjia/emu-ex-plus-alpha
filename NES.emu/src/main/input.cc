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

module;
#include <fceu/driver.h>
#include <fceu/fceu.h>
#include <fceu/fds.h>

module system;

namespace EmuEx
{

void NesSystem::connectNESInput(int port, ESI type)
{
	assume(GameInfo);
	if(type == SI_GAMEPAD)
	{
		//log.debug("gamepad to port {}", port);
		FCEUI_SetInput(port, SI_GAMEPAD, &padData, 0);
	}
	else if(type == SI_ZAPPER)
	{
		//log.debug("zapper to port {}", port);
		FCEUI_SetInput(port, SI_ZAPPER, &zapperData, 1);
	}
	else
	{
		FCEUI_SetInput(port, SI_NONE, 0, 0);
	}
}

static unsigned playerInputShift(int player)
{
	switch(player)
	{
		case 1: return 8;
		case 2: return 16;
		case 3: return 24;
	}
	return 0;
}

void NesSystem::handleInputAction(EmuApp *app, InputAction a)
{
	int player = a.flags.deviceId;
	auto key = NesKey(a.code);
	if(key == NesKey::toggleDiskSide)
	{
		if(!isFDS || !a.isPushed())
			return;
		EmuSystemTask::SuspendContext suspendCtx;
		if(app)
			suspendCtx = app->suspendEmulationThread();
		if(FCEU_FDSInserted())
		{
			FCEU_FDSInsert();
			if(app)
				app->postMessage("Disk ejected, push again to switch side");
		}
		else
		{
			FCEU_FDSSelect();
			FCEU_FDSInsert();
			auto fdsSideToString = [](uint8_t side)
			{
				switch(side)
				{
					case 0: return "Disk 1 Side A";
					case 1: return "Disk 1 Side B";
					case 2: return "Disk 2 Side A";
					case 3: return "Disk 2 Side B";
				}
				std::unreachable();
			};
			if(app)
				app->postMessage(std::format("Set {}", fdsSideToString(FCEU_FDSCurrentSide())));
		}
	}
	else // gamepad bits
	{
		auto gpBits = bit(a.code - 1);
		if(GameInfo->type == GIT_NSF && a.isPushed())
		{
			if(key == NesKey::Up)
				FCEUI_NSFChange(10);
			else if(key == NesKey::Down)
				FCEUI_NSFChange(-10);
			else if(key == NesKey::Right)
				FCEUI_NSFChange(1);
			else if(key == NesKey::Left)
				FCEUI_NSFChange(-1);
			else if(key == NesKey::B)
				FCEUI_NSFChange(0);
		}
		else if(GameInfo->type == GIT_VSUNI) // TODO: make coin insert separate key
		{
			if(a.isPushed() && key == NesKey::Start)
				FCEUI_VSUniCoin();
		}
		else if(GameInfo->inputfc == SIFC_HYPERSHOT)
		{
			if(auto hsKey = gpBits & 0x3;
				hsKey)
			{
				hsKey = hsKey == 0x3 ? 0x3 : hsKey ^ 0x3; // swap the 2 bits
				auto hsPlayerInputShift = player == 1 ? 3 : 1;
				fcExtData = setOrClearBits(fcExtData, hsKey << hsPlayerInputShift, a.isPushed());
			}
		}
		padData = setOrClearBits(padData, gpBits << playerInputShift(player), a.isPushed());
	}
}

bool NesSystem::onPointerInputStart(const Input::MotionEvent &e, Input::DragTrackerState, WRect gameRect)
{
	if(!usingZapper)
		return false;
	zapperData[2] = 0;
	if(gameRect.overlaps(e.pos()))
	{
		int xRel = e.pos().x - gameRect.x, yRel = e.pos().y - gameRect.y;
		int xNes = remap(xRel, 0, gameRect.xSize(), 0, 256);
		int yNes = remap(yRel, 0, gameRect.ySize(), 0, 224) + 8;
		log.info("zapper pushed @ {},{}, on NES {},{}", e.pos().x, e.pos().y, xNes, yNes);
		zapperData[0] = xNes;
		zapperData[1] = yNes;
		zapperData[2] |= 0x1;
	}
	else // off-screen shot
	{
		zapperData[0] = 0;
		zapperData[1] = 0;
		zapperData[2] |= 0x2;
	}
	return true;
}

bool NesSystem::onPointerInputEnd(const Input::MotionEvent &, Input::DragTrackerState, WRect)
{
	if(!usingZapper)
		return false;
	zapperData[2] = 0;
	return true;
}

void NesSystem::clearInputBuffers()
{
	fill(zapperData);
	padData = {};
	fcExtData = {};
}

}

extern "C++" void GetMouseData(uint32 (&d)[3])
{
	// TODO
}
