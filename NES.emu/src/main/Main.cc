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
#include <fceu/ppu.h>
#include <fceu/fds.h>
#include <fceu/input.h>
#include <fceu/cart.h>
#include <fceu/video.h>
#include <fceu/sound.h>
#include <fceu/x6502.h>
#include <fceu/palette.h>
#include <fceu/state.h>
#undef PS

module system;
import io;

extern "C++"
{
	void ApplyDeemphasisComplete(pal* pal512);
	void FCEU_setDefaultPalettePtr(pal*);
	void ApplyIPS(FILE* ips, FCEUFILE*);
	FCEUGI* FCEUI_LoadGameWithFileVirtual(FCEUFILE*, const char* name, int OverwriteVidMode, bool silent);
}

namespace EmuEx
{

NesSystem::NesSystem(ApplicationContext ctx):
	EmuSystem{ctx}
{
	XBuf = XBufData;
	XBackBuf = XBufData;
	backupSavestates = false;
	if(!FCEUI_Initialize())
	{
		throw std::runtime_error{"Error in FCEUI_Initialize"};
	}
}

extern "C++" std::string_view EmuSystem::shortSystemName() const { return "FC-NES"; }
extern "C++" std::string_view EmuSystem::systemName() const { return "Famicom (Nintendo Entertainment System)"; }

void NesSystem::reset(EmuApp &app, ResetMode mode)
{
	assume(hasContent());
	if(mode == ResetMode::HARD)
		FCEUI_PowerNES();
	else
		FCEUI_ResetNES();
}

static char saveSlotCharNES(int slot)
{
	switch(slot)
	{
		case -1: return 's';
		case 0 ... 9: return '0' + slot;
		default: unreachable();
	}
}

//region 爱吾修改
std::string NesSystem::getSystemSettingValueAiWu(std::string_view key)
{
	if (key == "palette")
		return defaultPalettePath;
	return "";
}
void NesSystem::setSystemSettingValueAiWu(std::string_view key, std::string_view value)
{
	if (key == "palette"){
		if(!value.empty())
			defaultPalettePath = std::string(value.data(), value.size());
		else
			defaultPalettePath = {};
		setDefaultPalette(appContext(), defaultPalettePath);
	}
}
//endregion

FS::FileString NesSystem::stateFilename(int slot, std::string_view name) const
{
	return format<FS::FileString>("{}.fc{}", name, saveSlotCharAiWu(slot));//爱吾修改：修改即时存档插槽名
}

void NesSystem::readState(EmuApp &app, std::span<uint8_t> buff)
{
	EmuFileIO memFile{buff};
	if(!FCEUSS_LoadFP(&memFile, SSLOADPARAM_NOBACKUP))
		throw std::runtime_error("Invalid state data");
}

size_t NesSystem::writeState(std::span<uint8_t> buff, SaveStateFlags flags)
{
	assume(buff.size() >= saveStateSize);
	EmuFileIO memFile{buff};
	FCEUSS_SaveMS(&memFile, -1);
	return memFile.ftell();
}

void NesSystem::loadBackupMemory(EmuApp &app)
{
	if(isFDS)
	{
		FCEU_FDSReadModifiedDisk();
	}
	else if(currCartInfo)
	{
		FCEU_LoadGameSave(currCartInfo);
	}
}

void NesSystem::onFlushBackupMemory(EmuApp &, BackupMemoryDirtyFlags)
{
	if(isFDS)
	{
		FCEU_FDSWriteModifiedDisk();
	}
	else if(currCartInfo)
	{
		FCEU_SaveGameSave(currCartInfo);
	}
}

WallClockTimePoint NesSystem::backupMemoryLastWriteTime(const EmuApp &app) const
{
	return appContext().fileUriLastWriteTime(
		app.contentSaveFilePath(isFDS ? ".fds.sav" : ".sav").c_str());
}

void NesSystem::closeSystem()
{
	FCEUI_CloseGame();
	fdsIsAccessing = false;
}

void NesSystem::setDefaultPalette(IO &io)
{
	auto colors = io.read(std::span<pal>{defaultPal}).items;
	if(colors < 64)
	{
		log.error("skipped palette with only {} colors", colors);
		return;
	}
	if(colors != 512)
	{
		ApplyDeemphasisComplete(defaultPal.data());
	}
	FCEU_setDefaultPalettePtr(defaultPal.data());
}

void NesSystem::setDefaultPalette(ApplicationContext ctx, CStringView palPath)
{
	if(palPath.empty())
	{
		FCEU_setDefaultPalettePtr(nullptr);
		return;
	}
	try
	{
		log.info("setting default palette with path:{}", palPath);
		if(palPath[0] != '/' && !isUri(palPath))
		{
			// load as asset
			IO io = ctx.openAsset(FS::pathString("palette", palPath), {.accessHint = IOAccessHint::All});
			setDefaultPalette(io);
		}
		else
		{
			IO io = ctx.openFileUri(palPath, {.accessHint = IOAccessHint::All});
			setDefaultPalette(io);
		}
	}
	catch(...)
	{
		FCEU_setDefaultPalettePtr(nullptr);
	}
}

void NesSystem::cacheUsingZapper()
{
	assume(GameInfo);
	for(const auto &p : joyports)
	{
		if(p.type == SI_ZAPPER)
		{
			usingZapper = true;
			return;
		}
	}
	usingZapper = GameInfo->inputfc == SIFC_SHADOW;
}

static const char* fceuInputToStr(int input)
{
	switch(input)
	{
		case SI_UNSET: return "Unset";
		case SI_GAMEPAD: return "Gamepad";
		case SI_ZAPPER: return "Zapper";
		case SI_NONE: return "None";
		default: unreachable();
	}
}

void NesSystem::setupNESFourScore()
{
	if(!GameInfo)
		return;
	if(!usingZapper)
	{
		if(optionFourScore)
			log.info("attaching four score");
		FCEUI_SetInputFourscore(optionFourScore);
	}
	else
		FCEUI_SetInputFourscore(0);
}

VideoSystem NesSystem::videoSystem() const
{
	return PAL || dendy ? VideoSystem::PAL : VideoSystem::NATIVE_NTSC;
}

double NesSystem::videoAspectRatioScale() const
{
	double horizontalCropScaler = 240. / 256.; // cropped width / full pixel width
	double baseLines = 224.;
	assume(optionVisibleVideoLines != 0);
	double lineAspectScaler = baseLines / optionVisibleVideoLines;
	return (optionCorrectLineAspect ? lineAspectScaler : 1.)
		* (optionHorizontalVideoCrop ? horizontalCropScaler : 1.);
}

void NesSystem::setupNESInputPorts()
{
	if(!GameInfo)
		return;
	for(auto &&[i, dev] : enumerate(std::array{inputPort1.value(), inputPort2.value()}))
	{
		if(dev == SI_UNSET) // user didn't specify device, go with auto settings
			connectNESInput(i, GameInfo->input[i] == SI_UNSET ? SI_GAMEPAD : GameInfo->input[i]);
		else
			connectNESInput(i, dev);
		log.info("attached {} to port {}{}", fceuInputToStr(joyports[i].type), i, dev == SI_UNSET ? " (auto)" : "");
	}
	if(GameInfo->inputfc == SIFC_HYPERSHOT)
	{
		FCEUI_SetInputFC(SIFC_HYPERSHOT, &fcExtData, 0);
	}
	else if(GameInfo->inputfc == SIFC_SHADOW)
	{
		FCEUI_SetInputFC(SIFC_SHADOW, &zapperData, 0);
	}
	else
	{
		FCEUI_SetInputFC(SIFC_NONE, nullptr, 0);
	}
	cacheUsingZapper();
	setupNESFourScore();
}

const char *regionToStr(int region)
{
	switch(region)
	{
		case 0: return "NTSC";
		case 1: return "PAL";
		case 2: return "Dendy";
	}
	return "Unknown";
}

static int regionFromName(std::string_view name)
{
	if(containsAny(name, "(E)", "(e)", "(EU)", "(Europe)", "(PAL)",
		"(F)", "(f)", "(G)", "(g)", "(I)", "(i)"))
	{
		return 1; // PAL
	}
	else if(containsAny(name, "(RU)", "(ru)"))
	{
		return 2; // Dendy
	}
	return 0; // NTSC
}

void setRegion(int region, int defaultRegion, int detectedRegion)
{
	if(region)
	{
		NesSystem::log.info("Forced region:{}", regionToStr(region - 1));
		FCEUI_SetRegion(region - 1, false);
	}
	else if(defaultRegion)
	{
		NesSystem::log.info("Forced region (Default):{}", regionToStr(defaultRegion - 1));
		FCEUI_SetRegion(defaultRegion - 1, false);
	}
	else
	{
		NesSystem::log.info("Detected region:{}", regionToStr(detectedRegion));
		FCEUI_SetRegion(detectedRegion, false);
	}
}

void NesSystem::loadContent(IO &io, EmuSystemCreateParams, OnLoadProgressDelegate)
{
	auto ioStream = new EmuFileIO(io);
	auto file = new FCEUFILE();
	file->filename = contentFileName();
	file->archiveIndex = -1;
	file->stream = ioStream;
	file->size = ioStream->size();
	if(auto ipsFile = FileUtils::fopenUri(appContext(), userFilePath(patchesDir, ".ips"), "r");
		ipsFile)
	{
		ApplyIPS(ipsFile, file);
	}
	if(!FCEUI_LoadGameWithFileVirtual(file, contentFileName().data(), 0, false))
	{
		if(loaderErrorString.size())
			throw std::runtime_error(std::exchange(loaderErrorString, {}));
		else
			throw std::runtime_error("Error loading game");
	}
	autoDetectedRegion = regionFromName(contentFileName());
	setRegion(optionVideoSystem, optionDefaultVideoSystem, autoDetectedRegion);
	setupNESInputPorts();
	EMUFILE_MEMORY stateMemFile;
	FCEUSS_SaveMS(&stateMemFile, 0);
	saveStateSize = stateMemFile.get_vec()->size();
}

bool NesSystem::onVideoRenderFormatChange(EmuVideo &video, PixelFormat fmt)
{
	pixFmt = fmt;
	updateVideoPixmap(video, optionHorizontalVideoCrop, optionVisibleVideoLines);
	FCEU_ResetPalette();
	return true;
}

void NesSystem::configAudioRate(FrameRate outputFrameRate, int outputRate)
{
	uint32 mixRate = std::round(audioMixRate(outputRate, outputFrameRate));
	if(FSettings.SndRate == mixRate)
		return;
	log.info("set sound mix rate:{}", (int)mixRate);
	FCEUI_Sound(mixRate);
}

void emulateSound(EmuAudio *audio)
{
	static constexpr size_t maxAudioFrames = 1024;
	int32 sound[maxAudioFrames];
	auto frames = FlushEmulateSound(sound);
	soundtimestamp = 0;
	//log.debug("{} frames", frames);
	assume(frames <= maxAudioFrames);
	if(audio)
	{
		int16 sound16[maxAudioFrames];
		copy_n(sound, maxAudioFrames, sound16);
		audio->writeFrames(sound16, frames);
	}
}

void NesSystem::updateVideoPixmap(EmuVideo &video, bool horizontalCrop, int lines)
{
	int xPixels = horizontalCrop ? 240 : 256;
	video.setFormat({{xPixels, lines}, pixFmt});
}

void NesSystem::renderVideo(EmuSystemTaskContext taskCtx, EmuVideo &video, uint8 *buf)
{
	auto img = video.startFrame(taskCtx);
	auto pix = img.pixmap();
	PixmapView ppuPix{{{256, 256}, PixelFmtI8}, buf};
	int xStart = pix.w() == 256 ? 0 : 8;
	int yStart = optionStartVideoLine;
	auto ppuPixRegion = ppuPix.subView({xStart, yStart}, pix.size());
	assume(pix.size() == ppuPixRegion.size());
	if(pix.format() == PixelFmtRGB565)
	{
		pix.writeTransformed([&](uint8 p){ return nativeCol.col16[p]; }, ppuPixRegion);
	}
	else
	{
		assume(pix.format().bytesPerPixel() == 4);
		pix.writeTransformed([&](uint8 p){ return nativeCol.col32[p]; }, ppuPixRegion);
	}
	img.endFrame();
}

void NesSystem::runFrame(EmuSystemTaskContext taskCtx, EmuVideo *video, EmuAudio *audio)
{
	bool skip = !video && !optionCompatibleFrameskip;
	FCEUI_Emulate(taskCtx, static_cast<NesSystemHolder&>(*this), video, skip, audio);
}

void NesSystem::renderFramebuffer(EmuVideo &video)
{
	renderVideo({}, video, XBuf);
}

bool NesSystem::shouldFastForward() const
{
	return fdsIsAccessing;
}

}

extern "C++" void FCEUD_SetPalette(uint8 index, uint8 r, uint8 g, uint8 b)
{
	using namespace EmuEx;
	auto &sys = static_cast<NesSystem&>(gSystem());
	if(sys.pixFmt == PixelFmtRGB565)
	{
		sys.nativeCol.col16[index] = sys.pixFmt.desc().build(r >> 3, g >> 2, b >> 3, 0);
	}
	else // RGBA8888
	{
		auto desc = sys.pixFmt == PixelFmtBGRA8888 ? PixelDescBGRA8888Native : PixelDescRGBA8888Native;
		sys.nativeCol.col32[index] = desc.build(r, g, b, (uint8)0);
	}
	//log.debug("set palette {} {}", index, nativeCol[index]);
}

extern "C++" void FCEUPPU_FrameReady(EmuEx::EmuSystemTaskContext taskCtx, EmuEx::NesSystemHolder& sys, EmuEx::EmuVideo* video, uint8* buf)
{
	if(!video)
	{
		return;
	}
	if(!buf) [[unlikely]]
	{
		video->startUnchangedFrame(taskCtx);
		return;
	}
	sys.renderVideo(taskCtx, *video, buf);
}

extern "C++" void setDiskIsAccessing(bool on)
{
	using namespace EmuEx;
	auto &sys = static_cast<NesSystem&>(gSystem());
	if(sys.fastForwardDuringFdsAccess)
	{
		//if(on && !sys.fdsIsAccessing) log.debug("FDS access started");
		sys.fdsIsAccessing = on;
	}
	else
	{
		sys.fdsIsAccessing = false;
	}
}
