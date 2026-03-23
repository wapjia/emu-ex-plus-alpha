/*  This file is part of PCE.emu.

	PCE.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	PCE.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with PCE.emu.  If not, see <http://www.gnu.org/licenses/> */

module;
#include <mednafen/mednafen.h>
#include <mednafen/cdrom/CDInterface.h>
#include <mednafen/state-driver.h>
#include <mednafen/hash/md5.h>
#include <mednafen/MemoryStream.h>
#include "mdfnDefs.hh"
#include <pce/huc.h>
#include <pce_fast/huc.h>

module system;

namespace EmuEx
{

constexpr double masterClockFrac = 21477272.727273 / 3.;
constexpr FrameRate pceFrameRateWith262Lines{masterClockFrac / (455. * 262.)}; // ~60.05Hz
constexpr FrameRate pceFrameRate{masterClockFrac / (455. * 263.)}; // ~59.82Hz

extern "C++" std::string_view EmuSystem::shortSystemName() const { return "PCE-TG16"; }
extern "C++" std::string_view EmuSystem::systemName() const { return "PC Engine (TurboGrafx-16)"; }

void PceSystem::loadBackupMemory(EmuApp &)
{
	log.info("loading backup memory");
	if(isUsingAccurateCore())
		MDFN_IEN_PCE::HuC_LoadNV();
	else
		MDFN_IEN_PCE_FAST::HuC_LoadNV();
}

void PceSystem::onFlushBackupMemory(EmuApp &, BackupMemoryDirtyFlags)
{
	if(!hasContent())
		return;
	log.info("saving backup memory");
	if(isUsingAccurateCore())
		MDFN_IEN_PCE::HuC_SaveNV();
	else
		MDFN_IEN_PCE_FAST::HuC_SaveNV();
}

WallClockTimePoint PceSystem::backupMemoryLastWriteTime(const EmuApp &app) const
{
	return appContext().fileUriLastWriteTime(savePathMDFN(app, 0, "sav", noMD5InFilenames).c_str());
}

FS::FileString PceSystem::stateFilename(int slot, std::string_view name) const
{
	//爱吾修改：修改即时存档插槽名
	//return stateFilenameMDFN(*MDFNGameInfo, slot, name, 'q', noMD5InFilenames);
	return IG::format<FS::FileString>("{}.nc{}", name, saveSlotCharAiWu(slot));
}

void PceSystem::closeSystem()
{
	mdfnGameInfo.CloseGame();
	clearCDInterfaces(CDInterfaces);
}

WSize PceSystem::multiresVideoBaseSize() const { return {512, 0}; }

void PceSystem::loadContent(IO &io, EmuSystemCreateParams, OnLoadProgressDelegate)
{
	mdfnGameInfo = resolvedCore() == EmuCore::Accurate ? EmulatedPCE : EmulatedPCE_Fast;
	log.info("using emulator core module:{}", asModuleString(resolvedCore()));
	if(hasCDExtension(contentFileName()))
	{
		bool isArchive = std::holds_alternative<ArchiveIO>(io);
		bool isCHD = endsWithAnyCaseless(contentFileName(), ".chd");
		if(contentDirectory().empty() && (!isArchive && !isCHD))
		{
			throwMissingContentDirError();
		}
		if(sysCardPath.empty() || !appContext().fileUriExists(sysCardPath))
		{
			throw std::runtime_error("No System Card Set");
		}
		auto unloadCD = scopeGuard([&]() { clearCDInterfaces(CDInterfaces); });
		if(isArchive)
		{
			ArchiveVFS archVFS{ArchiveIO{std::move(io)}};
			CDInterfaces.push_back(CDInterface::Open(&archVFS, std::string{contentFileName()}, true, 0));
		}
		else
		{
			CDInterfaces.push_back(CDInterface::Open(&NVFS, std::string{contentLocation()}, false, 0));
		}
		writeCDMD5(mdfnGameInfo, CDInterfaces);
		mdfnGameInfo.LoadCD(&CDInterfaces);
		if(isUsingAccurateCore())
			Mednafen::SCSICD_SetDisc(false, CDInterfaces[0]);
		else
			MDFN_IEN_PCE_FAST::PCECD_Drive_SetDisc(false, CDInterfaces[0]);
		unloadCD.cancel();
	}
	else
	{
		static constexpr size_t maxRomSize = 0x300000;
		EmuEx::loadContent(*this, mdfnGameInfo, io, maxRomSize);
	}
	//logMsg("%d input ports", MDFNGameInfo->InputInfo->InputPorts);
	for(auto i: iotaCount(5))
	{
		mdfnGameInfo.SetInput(i, "gamepad", (uint8*)&inputBuff[i]);
	}
	updatePixmap(mSurfacePix.format());
}

bool PceSystem::onVideoRenderFormatChange(EmuVideo &, PixelFormat fmt)
{
	updatePixmap(fmt);
	return false;
}

void PceSystem::updatePixmap(PixelFormat fmt)
{
	mSurfacePix = {{{mdfnGameInfo.fb_width, mdfnGameInfo.fb_height}, fmt}, pixBuff};
	if(!hasContent())
		return;
	if(isUsingAccurateCore())
		MDFN_IEN_PCE::vce->SetPixelFormat(toMDFNSurface(mSurfacePix).format, nullptr, 0);
	else
		MDFN_IEN_PCE_FAST::VDC_SetPixelFormat(toMDFNSurface(mSurfacePix).format, nullptr, 0);
	return;
}

FrameRate PceSystem::frameRate() const { return isUsing263Lines() ? pceFrameRate : pceFrameRateWith262Lines; }

void PceSystem::configAudioRate(FrameRate outputFrameRate, int outputRate)
{
	configuredFor263Lines = isUsing263Lines();
	auto mixRate = audioMixRate(outputRate, outputFrameRate);
	if(!isUsingAccurateCore())
		mixRate = std::round(mixRate);
	auto currMixRate = isUsingAccurateCore() ? MDFN_IEN_PCE::GetSoundRate() : MDFN_IEN_PCE_FAST::GetSoundRate();
	if(mixRate == currMixRate)
		return;
	log.info("set sound mix rate:{} for {} video lines", mixRate, isUsing263Lines() ? "263" : "262");
	if(isUsingAccurateCore())
		MDFN_IEN_PCE::SetSoundRate(mixRate);
	else
		MDFN_IEN_PCE_FAST::SetSoundRate(mixRate);
}

void PceSystem::runFrame(EmuSystemTaskContext taskCtx, EmuVideo *video, EmuAudio *audio)
{
	static constexpr size_t maxAudioFrames = 48000 / AppMeta::minFrameRate;
	static constexpr size_t maxLineWidths = 264;
	EmuEx::runFrame(*this, mdfnGameInfo, taskCtx, video, mSurfacePix, audio, maxAudioFrames, maxLineWidths);
	if(configuredFor263Lines != isUsing263Lines()) [[unlikely]]
	{
		onFrameRateChanged();
	}
}

void PceSystem::reset(EmuApp &, ResetMode mode)
{
	assume(hasContent());
	mdfnGameInfo.DoSimpleCommand(MDFN_MSC_RESET);
}

size_t PceSystem::stateSize() { return stateSizeMDFN(); }
void PceSystem::readState(EmuApp&, std::span<uint8_t> buff) { readStateMDFN(buff); }
size_t PceSystem::writeState(std::span<uint8_t> buff, SaveStateFlags flags) { return writeStateMDFN(buff, flags); }

double PceSystem::videoAspectRatioScale() const
{
	double baseLines = 224.;
	double lineCount = (visibleLines.last - visibleLines.first) + 1;
	assume(lineCount >= 0);
	double lineAspectScaler = baseLines / lineCount;
	return correctLineAspect ? lineAspectScaler : 1.;
}

}

extern "C++" namespace Mednafen
{

using namespace IG;

void MDFN_MidSync(EmulateSpecStruct *espec, const unsigned flags)
{
	if(!espec->audio)
		return;
	espec->audio->writeFrames(espec->SoundBuf, std::exchange(espec->SoundBufSize, 0));
}

template <class Pixel>
static void renderMultiresOutput(EmulateSpecStruct spec, PixmapView srcPix, int multiResOutputWidth)
{
	int pixHeight = spec.DisplayRect.h;
	auto img = spec.video->startFrameWithFormat(spec.taskCtx, {{multiResOutputWidth, pixHeight}, srcPix.format()});
	auto destPixAddr = (Pixel*)img.pixmap().data();
	auto lineWidth = spec.LineWidths + spec.DisplayRect.y;
	if(multiResOutputWidth == 1024)
	{
		// scale 256x4, 341x3 + 1x4, 512x2
		for(auto h: iotaCount(pixHeight))
		{
			auto srcPixAddr = (Pixel*)&srcPix[0, h];
			int width = lineWidth[h];
			switch(width)
			{
				default:
					unreachable();
				case 256:
				{
					for([[maybe_unused]] auto w: iotaCount(256))
					{
						*destPixAddr++ = *srcPixAddr;
						*destPixAddr++ = *srcPixAddr;
						*destPixAddr++ = *srcPixAddr;
						*destPixAddr++ = *srcPixAddr++;
					}
					break;
				}
				case 341:
				{
					for([[maybe_unused]] auto w: iotaCount(340))
					{
						*destPixAddr++ = *srcPixAddr;
						*destPixAddr++ = *srcPixAddr;
						*destPixAddr++ = *srcPixAddr++;
					}
					*destPixAddr++ = *srcPixAddr;
					*destPixAddr++ = *srcPixAddr;
					*destPixAddr++ = *srcPixAddr;
					*destPixAddr++ = *srcPixAddr++;
					break;
				}
				case 512:
				{
					for([[maybe_unused]] auto w: iotaCount(512))
					{
						*destPixAddr++ = *srcPixAddr;
						*destPixAddr++ = *srcPixAddr++;
					}
					break;
				}
			}
			destPixAddr += img.pixmap().paddingPixels();
		}
	}
	else // 512 width
	{
		for(auto h: iotaCount(pixHeight))
		{
			auto srcPixAddr = (Pixel*)&srcPix[0, h];
			int width = lineWidth[h];
			switch(width)
			{
				default:
					unreachable();
				case 256:
				{
					for([[maybe_unused]] auto w: iotaCount(256))
					{
						*destPixAddr++ = *srcPixAddr;
						*destPixAddr++ = *srcPixAddr++;
					}
					break;
				}
				case 512:
				{
					memcpy(destPixAddr, srcPixAddr, 512 * sizeof(Pixel));
					destPixAddr += 512;
					srcPixAddr += 512;
					break;
				}
			}
			destPixAddr += img.pixmap().paddingPixels();
		}
	}
	img.endFrame();
}

void MDFND_commitVideoFrame(EmulateSpecStruct *espec)
{
	const auto spec = *espec;
	int pixHeight = spec.DisplayRect.h;
	bool uses256 = false;
	bool uses341 = false;
	bool uses512 = false;
	for(int i = spec.DisplayRect.y; i < spec.DisplayRect.y + pixHeight; i++)
	{
		int w = spec.LineWidths[i];
		assume(w == 256 || w == 341 || w == 512);
		switch(w)
		{
			case 256: uses256 = true; break;
			case 341: uses341 = true; break;
			case 512: uses512 = true; break;
		}
	}
	int pixWidth = 256;
	int multiResOutputWidth = 0;
	if(uses512)
	{
		pixWidth = 512;
		if(uses341)
		{
			multiResOutputWidth = 1024;
		}
		else if(uses256)
		{
			multiResOutputWidth = 512;
		}
	}
	else if(uses341)
	{
		pixWidth = 341;
		if(uses256)
		{
			multiResOutputWidth = 1024;
		}
	}
	PixmapView srcPix = static_cast<EmuEx::PceSystem&>(*espec->sys).mSurfacePix.subView(
		{spec.DisplayRect.x, spec.DisplayRect.y},
		{pixWidth, pixHeight});
	if(multiResOutputWidth)
	{
		if(srcPix.format() == PixelFmtRGB565)
			renderMultiresOutput<uint16_t>(spec, srcPix, multiResOutputWidth);
		else
			renderMultiresOutput<uint32_t>(spec, srcPix, multiResOutputWidth);
	}
	else
	{
		spec.video->startFrameWithFormat(espec->taskCtx, srcPix);
	}
}

}

extern "C++" namespace MDFN_IEN_PCE_FAST
{
// dummy HES functions
int PCE_HESLoad(const uint8 *buf, uint32 size) { return 0; };
void HES_Draw(MDFN_Surface *surface, MDFN_Rect *DisplayRect, int16 *SoundBuf, int32 SoundBufSize) {}
void HES_Close(void) {}
void HES_Reset(void) {}
uint8 ReadIBP(unsigned int A) { return 0; }
};
