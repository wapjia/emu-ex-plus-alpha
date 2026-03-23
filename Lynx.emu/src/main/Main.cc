/*  This file is part of Lynx.emu.

	Lynx.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Lynx.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Lynx.emu.  If not, see <http://www.gnu.org/licenses/> */

module;
#include <mednafen/mednafen.h>
#include <mednafen/state-driver.h>
#include <mednafen/hash/md5.h>
#include <mednafen/MemoryStream.h>
#include <mednafen/video/surface.h>

module system;

extern "C++"
{
	void Lynx_SetPixelFormat(Mednafen::MDFN_PixelFormat);
	int Lynx_HCount();
	bool Lynx_SetSoundRate(long rate);
	long Lynx_GetSoundRate();
}

namespace EmuEx
{

using namespace Mednafen;

extern "C++" std::string_view EmuSystem::shortSystemName() const { return "Lynx"; }
extern "C++" std::string_view EmuSystem::systemName() const { return "Lynx"; }

void LynxSystem::reset(EmuApp&, ResetMode)
{
	assume(hasContent());
	MDFN_DoSimpleCommand(MDFN_MSC_RESET);
}

FS::FileString LynxSystem::stateFilename(int slot, std::string_view name) const
{
	return stateFilenameMDFN(*MDFNGameInfo, slot, name, 'a', noMD5InFilenames);
}

size_t LynxSystem::stateSize() { return stateSizeMDFN(); }
void LynxSystem::readState(EmuApp&, std::span<uint8_t> buff) { readStateMDFN(buff); }
size_t LynxSystem::writeState(std::span<uint8_t> buff, SaveStateFlags flags) { return writeStateMDFN(buff, flags); }

void LynxSystem::closeSystem()
{
	mdfnGameInfo.CloseGame();
	mdfnGameInfo.rotated = MDFN_ROTATE0;
}

void LynxSystem::loadContent(IO &io, EmuSystemCreateParams, OnLoadProgressDelegate)
{
	static constexpr size_t maxRomSize = 0x1000000;
	EmuEx::loadContent(*this, mdfnGameInfo, io, maxRomSize);
	Lynx_SetPixelFormat(toMDFNSurface(mSurfacePix).format);
}

bool LynxSystem::onVideoRenderFormatChange(EmuVideo&, PixelFormat fmt)
{
	mSurfacePix = {{vidBufferPx, fmt}, pixBuff};
	if(!hasContent())
		return false;
	Lynx_SetPixelFormat(toMDFNSurface(mSurfacePix).format);
	return false;
}

static auto microsecondsPerFrame()
{
	static constexpr int linesPerFrame = 105;
	return Microseconds{Lynx_HCount() * linesPerFrame};
}

FrameRate LynxSystem::frameRate() const { return FrameRate{microsecondsPerFrame()}; }

void LynxSystem::configAudioRate(FrameRate outputFrameRate, int outputRate)
{
	long mixRate = std::round(audioMixRate(outputRate, outputFrameRate));
	configuredHCount = Lynx_HCount();
	if(Lynx_GetSoundRate() == mixRate)
		return;
	log.info("set sound mix rate:{}", mixRate);
	Lynx_SetSoundRate(mixRate);
}

void LynxSystem::runFrame(EmuSystemTaskContext taskCtx, EmuVideo* video, EmuAudio* audio)
{
	static constexpr size_t maxAudioFrames = 48000 / 20; // May output a large amount of audio samples during boot
	EmuEx::runFrame(*this, mdfnGameInfo, taskCtx, video, mSurfacePix, audio, maxAudioFrames);
	if(configuredHCount != Lynx_HCount()) [[unlikely]]
	{
		onFrameRateChanged();
	}
}

Rotation LynxSystem::contentRotation() const
{
	switch(rotation)
	{
		case LynxRotation::Auto:
			switch(mdfnGameInfo.rotated)
			{
				default: return Rotation::UP;
				case Mednafen::MDFN_ROTATE90: return Rotation::RIGHT;
				case Mednafen::MDFN_ROTATE270: return Rotation::LEFT;
			}
		case LynxRotation::Horizontal: return Rotation::UP;
		case LynxRotation::VerticalLeft: return Rotation::LEFT;
		case LynxRotation::VerticalRight: return Rotation::RIGHT;
	}
	unreachable();
}

}

extern "C++" namespace Mednafen
{

void MDFND_commitVideoFrame(EmulateSpecStruct* espec)
{
	espec->video->startFrameWithFormat(espec->taskCtx, static_cast<EmuEx::LynxSystem&>(*espec->sys).mSurfacePix);
}

}
