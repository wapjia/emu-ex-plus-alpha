/*  This file is part of GBA.emu.

	GBA.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	GBA.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with GBA.emu.  If not, see <http://www.gnu.org/licenses/> */

module;
#include <core/gba/gba.h>
#include <core/gba/gbaGfx.h>
#include <core/gba/gbaSound.h>
#include <core/gba/gbaRtc.h>
#include <core/gba/gbaEeprom.h>
#include <core/gba/gbaFlash.h>
#include <core/gba/gbaCheats.h>
#include <core/base/sound_driver.h>
#include <core/base/patch.h>
#include <core/base/file_util.h>
#include <sys/mman.h>

module system;

struct GameSettings
{
	std::string_view gameName;
	std::string_view gameId;
	int saveSize;
	int saveType;
	bool rtcEnabled;
	bool mirroringEnabled;
	bool useBios;
};

constexpr GameSettings settings[]
{
#include "gba-over.inc"
};

namespace EmuEx
{

constexpr WSize lcdSize{240, 160};
constexpr size_t stateSizeVer10 = 734424;
constexpr std::array validStateSizes{stateSizeVer10, stateSizeVer11};
static_assert(SAVE_GAME_VERSION == 11, "Update valid state sizes for new state version");

extern "C++" std::string_view EmuSystem::shortSystemName() const { return "GBA"; }
extern "C++" std::string_view EmuSystem::systemName() const { return "Game Boy Advance"; }

void GbaSystem::reset(EmuApp &, ResetMode mode)
{
	assume(hasContent());
	CPUReset(gGba);
}

FS::FileString GbaSystem::stateFilename(int slot, std::string_view name) const
{
	return format<FS::FileString>("{}{}.sgm", name, saveSlotCharAiWu(slot));//爱吾修改：修改即时存档插槽名
}

void GbaSystem::readState(EmuApp &app, std::span<uint8_t> buff)
{
	DynArray<uint8_t> uncompArr;
	if(hasGzipHeader(buff))
	{
		uncompArr = uncompressGzipState(buff);
		buff = uncompArr;
	}
	if(!std::ranges::contains(validStateSizes, buff.size()))
		throw std::runtime_error("Invalid state size");
	if(!CPUReadState(gGba, buff.data()))
		throw std::runtime_error("Invalid state data");
}

size_t GbaSystem::writeState(std::span<uint8_t> buff, SaveStateFlags flags)
{
	assume(buff.size() >= saveStateSize);
	if(flags.uncompressed)
	{
		return CPUWriteState(gGba, buff.data());
	}
	else
	{
		auto stateArr = DynArray<uint8_t>(saveStateSize);
		CPUWriteState(gGba, stateArr.data());
		return compressGzip(buff, stateArr, Z_DEFAULT_COMPRESSION);
	}
}

void GbaSystem::loadBackupMemory(EmuApp &app)
{
	if(coreOptions.saveType == GBA_SAVE_NONE)
		return;
	app.setupStaticBackupMemoryFile(saveFileIO, ".sav", saveMemorySize(), 0xFF);
	auto buff = saveFileIO.buffer(IOBufferMode::Release);
	if(buff.isMappedFile())
		saveFileIO = {};
	saveMemoryIsMappedFile = buff.isMappedFile();
	setSaveMemory(std::move(buff));
}

void GbaSystem::onFlushBackupMemory(EmuApp &app, BackupMemoryDirtyFlags)
{
	if(coreOptions.saveType == GBA_SAVE_NONE)
		return;
	const ByteBuffer &saveData = eepromInUse ? eepromData : flashSaveMemory;
	if(saveMemoryIsMappedFile)
	{
		log.info("flushing backup memory");
		msync(saveData.data(), saveData.size(), MS_SYNC);
	}
	else
	{
		log.info("saving backup memory");
		saveFileIO.write(saveData.span(), 0);
	}
}

WallClockTimePoint GbaSystem::backupMemoryLastWriteTime(const EmuApp &app) const
{
	return appContext().fileUriLastWriteTime(app.contentSaveFilePath(".sav").c_str());
}

void GbaSystem::closeSystem()
{
	assume(hasContent());
	CPUCleanUp();
	saveFileIO = {};
	coreOptions.saveType = GBA_SAVE_NONE;
	detectedRtcGame = 0;
	detectedSensorType = {};
	sensorListener = {};
	darknessLevel = darknessLevelDefault;
	cheatsList.clear();
  gGba.cpu.matrix = {};
  gGba.mem.rom2.reset();
}

void GbaSystem::applyGamePatches(uint8_t *rom, int &romSize)
{
	auto ctx = appContext();
	// The patchApply* functions are responsible for closing the FILE
	if(auto f = FileUtils::fopenUri(ctx, userFilePath(patchesDir, ".ips"), "rb");
		f)
	{
		log.info("applying IPS patch:{}", userFilePath(patchesDir, ".ips"));
		if(!patchApplyIPS(f, &rom, &romSize))
		{
			throw std::runtime_error(std::format("Error applying IPS patch in:\n{}", patchesDir));
		}
	}
	else if(auto f = FileUtils::fopenUri(ctx, userFilePath(patchesDir, ".ups"), "rb");
		f)
	{
		log.info("applying UPS patch:{}", userFilePath(patchesDir, ".ups"));
		if(!patchApplyUPS(f, &rom, &romSize))
		{
			throw std::runtime_error(std::format("Error applying UPS patch in:\n{}", patchesDir));
		}
	}
	else if(auto f = FileUtils::fopenUri(ctx, userFilePath(patchesDir, ".ppf"), "rb");
		f)
	{
		log.info("applying UPS patch:{}", userFilePath(patchesDir, ".ppf"));
		if(!patchApplyPPF(f, &rom, &romSize))
		{
			throw std::runtime_error(std::format("Error applying PPF patch in:\n{}", patchesDir));
		}
	}
}

void GbaSystem::loadContent(IO &io, EmuSystemCreateParams, OnLoadProgressDelegate)
{
	coreOptions.cpuIsMultiBoot = endsWithAnyCaseless(contentFileName(), ".mb");
	int size = CPULoadRomWithIO(gGba, io, coreOptions.cpuIsMultiBoot ? LoadDestination::ram : LoadDestination::rom);
	if(!size)
	{
		throwFileReadError();
	}
	setGameSpecificSettings(gGba, size);
	applyGamePatches(gGba.mem.rom, size);
	ByteBuffer biosRom;
	if(shouldUseBios())
	{
		biosRom = appContext().openFileUri(biosPath, {.accessHint = IOAccessHint::All}).buffer(IOBufferMode::Release);
		if(biosRom.size() != 0x4000)
			throw std::runtime_error("BIOS size should be 16KB");
	}
	CPUInit(gGba, biosRom);
	CPUReset(gGba);
	//readCheatFile();//爱吾修改：去掉原版读取金手指逻辑
}

static void updateColorMap(auto &map, const PixelDesc &pxDesc)
{
	for(auto i: iotaCount(0x10000))
	{
		auto r = remap(i & 0x1f, 0, 0x1f, 0.f, 1.f);
		auto g = remap((i & 0x3e0) >> 5, 0, 0x1f, 0.f, 1.f);
		auto b = remap((i & 0x7c00) >> 10, 0, 0x1f, 0.f, 1.f);
		map[i] = pxDesc.build(r, g, b, 1.f);
	}
}

bool GbaSystem::onVideoRenderFormatChange(EmuVideo &video, PixelFormat fmt)
{
	log.info("updating system color maps");
	video.setFormat({lcdSize, fmt});
	if(fmt == PixelFmtRGB565)
		updateColorMap(gGba.lcd.systemColorMap.map16, PixelDescRGB565);
	else
		updateColorMap(gGba.lcd.systemColorMap.map32, fmt.desc().nativeOrder());
	return true;
}

void GbaSystem::renderFramebuffer(EmuVideo &video)
{
	systemDrawScreen(gGba.lcd, {}, video);
}

void GbaSystem::runFrame(EmuSystemTaskContext taskCtx, EmuVideo *video, EmuAudio *audio)
{
	CPULoop(gGba, taskCtx, video, audio);
}

void GbaSystem::configAudioRate(FrameRate outputFrameRate, int outputRate)
{
	long mixRate = std::round(audioMixRate(outputRate, outputFrameRate));
	log.info("set sound mix rate:{}", mixRate);
	soundSetSampleRate(gGba, mixRate);
}

void GbaSystem::onStart()
{
	setSensorActive(true);
}

void GbaSystem::onStop()
{
	setSensorActive(false);
}

static void resetGameSettings()
{
	//agbPrintEnable(0);
	rtcEnable(0);
	g_flashSize = SIZE_FLASH512;
	eepromSize = SIZE_EEPROM_512;
}

static GbaSensorType detectSensorType(std::string_view gameId)
{
	static constexpr std::string_view tiltIds[]{"KHPJ", "KYGJ", "KYGE", "KYGP"};
	if(std::ranges::contains(tiltIds, gameId))
	{
		GbaSystem::log.info("detected accelerometer sensor");
		return GbaSensorType::Accelerometer;
	}
	static constexpr std::string_view gyroIds[]{"RZWJ", "RZWE", "RZWP"};
	if(std::ranges::contains(gyroIds, gameId))
	{
		GbaSystem::log.info("detected gyroscope sensor");
		return GbaSensorType::Gyroscope;
	}
	static constexpr std::string_view lightIds[]{"U3IJ", "U3IE", "U3IP",
		"U32J", "U32E", "U32P", "U33J"};
	if(std::ranges::contains(lightIds, gameId))
	{
		GbaSystem::log.info("detected light sensor");
		return GbaSensorType::Light;
	}
	return GbaSensorType::None;
}

void GbaSystem::setGameSpecificSettings(GBASys &gba, int romSize)
{
	resetGameSettings();
	log.info("game id:{}{}{}{}", gba.mem.rom[0xac], gba.mem.rom[0xad], gba.mem.rom[0xae], gba.mem.rom[0xaf]);
	GameSettings foundSettings{};
	std::string_view gameId{(char*)&gba.mem.rom[0xac], 4};
	if(auto it = std::ranges::find_if(settings, [&](const auto &s){return s.gameId == gameId;});
		it != std::end(settings))
	{
		foundSettings = *it;
		log.info("found settings for:{} save type:{} save size:{} rtc:{} mirroring:{}",
			it->gameName, saveTypeStr(it->saveType, it->saveSize), it->saveSize, it->rtcEnabled, it->mirroringEnabled);
	}
	detectedRtcGame = foundSettings.rtcEnabled;
	detectedSaveType = foundSettings.saveType;
	detectedSaveSize = foundSettings.saveSize;
	detectedSensorType = detectSensorType(gameId);
	doMirroring(gba, foundSettings.mirroringEnabled);
	if(detectedSaveType == GBA_SAVE_AUTO)
	{
		flashDetectSaveType(gba.mem.rom, romSize);
		detectedSaveType = coreOptions.saveType;
		detectedSaveSize = coreOptions.saveType == GBA_SAVE_FLASH ? g_flashSize : 0;
		log.info("save type found from rom scan:{}", saveTypeStr(detectedSaveType, detectedSaveSize));
	}
	if(auto [type, size] = saveTypeOverride();
		type != GBA_SAVE_AUTO)
	{
		setSaveType(type, size);
		log.info("save type override:{}", saveTypeStr(type, size));
	}
	else
	{
		setSaveType(detectedSaveType, detectedSaveSize);
	}
	setRTC(optionRtcEmulation);
}

void GbaSystem::setSensorActive(bool on)
{
	using namespace IG;
	auto ctx = appContext();
	auto typeToSet = sensorType;
	if(sensorType == GbaSensorType::Auto)
		typeToSet = detectedSensorType;
	if(!on)
	{
		sensorListener = {};
	}
	else if(typeToSet == GbaSensorType::Accelerometer)
	{
		sensorListener = SensorListener{ctx, SensorType::Accelerometer, [this, ctx](SensorValues vals)
		{
			vals = ctx.remapSensorValuesForDeviceRotation(vals);
			sensorX = remap(vals[0], -9.807f, 9.807f, 1897, 2197);
			sensorY = remap(vals[1], -9.807f, 9.807f, 2197, 1897);
			//log.debug("updated accel:{},{}", sensorX, sensorY);
		}};
	}
	else if(typeToSet == GbaSensorType::Gyroscope)
	{
		sensorListener = SensorListener{ctx, SensorType::Gyroscope, [this, ctx](SensorValues vals)
		{
			vals = ctx.remapSensorValuesForDeviceRotation(vals);
			sensorZ = remap(vals[2], -20.f, 20.f, 1800, -1800);
			//log.debug("updated gyro:{}", sensorZ);
		}};
	}
	else if(typeToSet == GbaSensorType::Light)
	{
		sensorListener = SensorListener{ctx, SensorType::Light, [this](SensorValues vals)
		{
			if(!lightSensorScaleLux)
				darknessLevel = 0;
			else
				darknessLevel = remapClamp(vals[0], lightSensorScaleLux, 0.f, std::numeric_limits<decltype(darknessLevel)>{});
			//log.debug("updated light:{}", darknessLevel);
		}};
	}
}

void GbaSystem::clearSensorValues()
{
	sensorX = sensorY = sensorZ = 0;
}

}

extern "C++"
{

void systemDrawScreen(const GBALCD& lcd, EmuEx::EmuSystemTaskContext taskCtx, EmuEx::EmuVideo &video)
{
	using namespace EmuEx;
	auto img = video.startFrame(taskCtx);
	PixmapView framePix{{lcdSize, PixelFmtRGB565}, lcd.pix};
	assume(img.pixmap().size() == framePix.size());
	if(img.pixmap().format() == PixelFmtRGB565)
	{
		img.pixmap().writeTransformed([&](uint16_t p){ return lcd.systemColorMap.map16[p]; }, framePix);
	}
	else
	{
		assume(img.pixmap().format().bytesPerPixel() == 4);
		img.pixmap().writeTransformed([&](uint16_t p){ return lcd.systemColorMap.map32[p]; }, framePix);
	}
	img.endFrame();
}

void systemOnWriteDataToSoundBuffer(EmuEx::EmuAudio *audio, const uint16_t *finalWave, int length)
{
	if(audio)
	{
		int frames = length >> 1; // stereo samples
		//log.debug("{} audio frames", frames);
		audio->writeFrames(finalWave, frames);
	}
}

}
