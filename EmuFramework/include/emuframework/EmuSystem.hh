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

#include <emuframework/EmuTiming.hh>
#include <emuframework/VController.hh>
#include <emuframework/EmuInput.hh>
#include <emuframework/AppMeta.hh>
#ifndef IG_USE_MODULE_IMAGINE
#include <imagine/fs/FSUtils.hh>
#include <imagine/base/baseDefs.hh>
#include <imagine/base/ApplicationContext.hh>
#include <imagine/time/Time.hh>
#include <imagine/audio/Format.hh>
#include <imagine/util/rectangle2.h>
#include <imagine/util/memory/DynArray.hh>
#endif
#ifndef IG_USE_MODULE_STD
#include <string>
#include <string_view>
#endif
#include <list>

#ifndef IG_USE_MODULE_IMAGINE
namespace IG
{
class ApplicationContext;
class PixelFormat;
class IO;
class FileIO;
class MapIO;
}

namespace IG::Input
{
class Event;
class MotionEvent;
class DragTrackerState;
enum class Action : uint8_t;
}
#endif

namespace EmuEx
{

using namespace IG;
class EmuInputView;
class EmuSystemTaskContext;
class EmuAudio;
class EmuVideo;
class EmuApp;
struct EmuFrameDurationInfo;
class VControllerKeyboard;
class Cheat;
class CheatCode;

struct CheatCodeDesc
{
	const char* str{};
	unsigned flags{};
};

struct EmuSystemCreateParams
{
	uint8_t systemFlags;
};

enum class ConfigType : uint8_t
{
	MAIN, SESSION, CORE
};

enum class VideoSystem: uint8_t
{
	NATIVE_NTSC, PAL
};

enum class DeinterlaceMode: uint8_t
{
	Bob, Weave
};

using FrameDuration = Nanoseconds;

inline constexpr const char *optionUserPathContentToken = ":CONTENT:";

struct SaveStateFlags
{
	uint8_t uncompressed:1{};
};

class EmuSystem
{
public:
	enum class State
	{
		OFF,
		STARTING,
		PAUSED,
		ACTIVE
	};

	enum class LoadProgress : uint8_t
	{
		UNSET,
		FAILED,
		OK,
		SET,
		UPDATE
	};

	struct LoadProgressMessage
	{
		int intArg{};
		int intArg2{};
		int intArg3{};
		LoadProgress progress{LoadProgress::UNSET};

		constexpr LoadProgressMessage() = default;
		constexpr LoadProgressMessage(LoadProgress progress, int intArg, int intArg2, int intArg3):
			intArg{intArg}, intArg2{intArg2}, intArg3{intArg3}, progress{progress} {}
	};

	using OnLoadProgressDelegate = DelegateFunc<bool(int pos, int max, const char *label)>;
	using BackupMemoryDirtyFlags = uint8_t;
	enum class ResetMode: uint8_t { HARD, SOFT };

	EmuSystem(ApplicationContext ctx): appCtx{ctx} {}

	// required sub-class API functions
	void loadContent(IO &, EmuSystemCreateParams, OnLoadProgressDelegate);
	[[gnu::hot]] void runFrame(EmuSystemTaskContext task, EmuVideo *video, EmuAudio *audio);
	FS::FileString stateFilename(int slot, std::string_view name) const;
	std::string_view stateFilenameExt() const;
	size_t stateSize();
	void readState(EmuApp &, std::span<uint8_t> buff);
	size_t writeState(std::span<uint8_t> buff, SaveStateFlags = {});
	bool readConfig(ConfigType, MapIO &io, unsigned key);
	void writeConfig(ConfigType, FileIO &);
	void reset(EmuApp &, ResetMode mode);
	void clearInputBuffers();
	void handleInputAction(EmuApp *, InputAction);
	FrameRate frameRate() const;
	void configAudioRate(FrameDuration outputFrameDuration, int outputRate);

	// optional sub-class API functions
	void onStart();
	void onStop();
	void closeSystem();
	bool onPointerInputStart(const Input::MotionEvent&, Input::DragTrackerState, WindowRect gameRect);
	bool onPointerInputUpdate(const Input::MotionEvent&, Input::DragTrackerState current, Input::DragTrackerState previous, WindowRect gameRect);
	bool onPointerInputEnd(const Input::MotionEvent&, Input::DragTrackerState, WindowRect gameRect);
	void onVKeyboardShown(VControllerKeyboard &, bool shown);
	VController::KbMap vControllerKeyboardMap(VControllerKbMode mode);
	VideoSystem videoSystem() const;
	void renderFramebuffer(EmuVideo &);
	WSize multiresVideoBaseSize() const;
	double videoAspectRatioScale() const;
	bool onVideoRenderFormatChange(EmuVideo &, PixelFormat);
	static bool canRenderMultipleFormats() {return AppMeta::canRenderRGBA8888 && AppMeta::canRenderRGB565;}
	void loadBackupMemory(EmuApp &);
	void onFlushBackupMemory(EmuApp &, BackupMemoryDirtyFlags);
	WallClockTimePoint backupMemoryLastWriteTime(const EmuApp &) const;
	FS::FileString configName() const;
	void onOptionsLoaded();
	void onSessionOptionsLoaded(EmuApp &);
	bool resetSessionOptions(EmuApp &);
	void savePathChanged();
	bool shouldFastForward() const;
	FS::FileString contentDisplayNameForPath(CStringView path) const;
	Rotation contentRotation() const;
	void addThreadGroupIds(std::vector<ThreadId> &) const;
	Cheat* newCheat(EmuApp&, const char* name, CheatCodeDesc);
	bool setCheatName(Cheat&, const char* name);
	std::string_view cheatName(const Cheat&) const;
	void setCheatEnabled(Cheat&, bool on);
	bool isCheatEnabled(const Cheat&) const;
	bool addCheatCode(EmuApp&, Cheat*&, CheatCodeDesc);
	bool modifyCheatCode(EmuApp&, Cheat&, CheatCode&, CheatCodeDesc);
	Cheat* removeCheatCode(Cheat&, CheatCode&);
	bool removeCheat(Cheat&);
	void forEachCheat(DelegateFunc<bool(Cheat&, std::string_view)>);
	void forEachCheatCode(Cheat&, DelegateFunc<bool(CheatCode&, std::string_view)>);

	ApplicationContext appContext() const { return appCtx; }
	bool isActive() const { return state == State::ACTIVE; }
	bool isStarted() const { return state == State::ACTIVE || state == State::PAUSED; }
	bool isPaused() const { return state == State::PAUSED; }
	void loadState(EmuApp &, CStringView uri);
	void saveState(CStringView uri);
	DynArray<uint8_t> saveState();
	DynArray<uint8_t> uncompressGzipState(std::span<uint8_t> buff, size_t expectedSize = 0);
	bool stateExists(int slot) const;
	static std::string_view stateSlotName(int slot);
	std::string_view systemName() const;
	std::string_view shortSystemName() const;
	const auto &contentDirectory() const { return contentDirectory_; }
	FS::PathString contentDirectory(std::string_view name) const;
	FS::PathString contentFilePath(std::string_view ext) const;
	const auto &contentLocation() const { return contentLocation_; }
	FS::FileString contentNameExt(std::string_view ext) const
	{
		FS::FileString name{contentName_};
		name += ext;
		return name;
	}
	const auto &contentName() const { return contentName_; }
	FS::FileString contentFileName() const;
	std::string contentDisplayName() const;
	void setContentDisplayName(std::string_view name);
	FS::FileString contentDisplayNameForPathDefaultImpl(CStringView path) const;
	void setInitialLoadPath(CStringView path);
	FS::PathString fallbackSaveDirectory(bool create = false);
	const auto &contentSaveDirectory() const { return contentSaveDirectory_; }

	FS::PathString contentLocalSaveDirectory(auto &&...components) const
	{
		return contentLocalDirectory(contentSaveDirectory_, "saves", IG_forward(components)...);
	}

	bool createContentLocalSaveDirectory(auto &&...components)
	{
		return createContentLocalDirectory(contentSaveDirectory_, "saves", IG_forward(components)...);
	}

	FS::PathString contentLocalDirectory(std::string_view basePath, std::string_view name, auto &&...components) const
	{
		assume(!contentName_.empty());
		return FS::uriString(basePath, contentName_, name, IG_forward(components)...);
	}

	bool createContentLocalDirectory(std::string_view basePath, std::string_view name, auto &&...components)
	{
		assume(!contentName_.empty());
		try
		{
			FS::createDirectoryUriSegments(appContext(), basePath, contentName_, name, IG_forward(components)...);
		}
		catch(...)
		{
			return false;
		}
		return true;
	}

	FS::PathString contentSavePath(std::string_view name) const;
	const char *contentSaveDirectoryPtr() { return contentSaveDirectory_.data(); }
	FS::PathString contentSaveFilePath(std::string_view ext) const;
	const auto &userSaveDirectory() const { return userSaveDirectory_; }
	void setUserSaveDirectory(CStringView path);
	FS::FileString stateFilename(int slot) const { return stateFilename(slot, contentName_); }
	FS::FileString stateFilename(std::string_view name) const;
	FS::PathString statePath(std::string_view filename, std::string_view basePath) const;
	FS::PathString statePath(std::string_view filename) const;
	FS::PathString statePath(int slot, std::string_view basePath) const;
	FS::PathString statePath(int slot) const;
	FS::PathString userPath(std::string_view userDir, std::string_view filename) const;
	FS::PathString userPath(std::string_view userDir) const;
	FS::PathString userFilePath(std::string_view userDir, std::string_view ext) const;
	void clearGamePaths();
	char saveSlotChar(int slot) const;
	char saveSlotCharUpper(int slot) const;
	const char *saveSlotCharAiWu(int slot) const;//爱吾修改：修改即时存档插槽的名字
	void flushBackupMemory(EmuApp &, BackupMemoryDirtyFlags flags = 0xFF);
	void onBackupMemoryWritten(BackupMemoryDirtyFlags flags = 0xFF);
	bool updateBackupMemoryCounter();
	bool usesBackupMemory() const;
	FileIO openStaticBackupMemoryFile(CStringView uri, size_t staticSize, uint8_t initValue = 0) const;
	void sessionOptionSet();
	void resetSessionOptionsSet() { sessionOptionsSet = false; }
	bool sessionOptionsAreSet() const { return sessionOptionsSet; }
	void createWithMedia(IO, CStringView path,
		std::string_view displayName, EmuSystemCreateParams, OnLoadProgressDelegate);
	FS::PathString willLoadContentFromPath(std::string_view path, std::string_view displayName);
	void loadContentFromPath(CStringView path, std::string_view displayName,
		EmuSystemCreateParams, OnLoadProgressDelegate);
	void loadContentFromFile(IO, CStringView path, std::string_view displayName,
		EmuSystemCreateParams, OnLoadProgressDelegate);
	int updateAudioFramesPerVideoFrame();
	FrameRate scaledFrameRate() const
	{
		if(frameRateMultiplier == 1.)
		{
			return frameRate();
		}
		else
		{
			return frameRate().hz() * frameRateMultiplier;
		}
	}
	void onFrameRateChanged();
	static double audioMixRate(int outputRate, FrameRate inputFrameRate, FrameRate outputFrameRate);
	double audioMixRate(int outputRate, FrameRate outputFrameRate) const { return audioMixRate(outputRate, frameRate(), outputFrameRate); }
	void configFrameRate(int outputRate, FrameDuration outputFrameDuration);
	SteadyClockDuration benchmark(EmuVideo&);
	bool hasContent() const;
	void resetFrameTiming();
	void pause(EmuApp &);
	void start(EmuApp &);
	void closeRuntimeSystem(EmuApp &);
	void runFrames(EmuSystemTaskContext, EmuVideo*, EmuAudio*, int frames);
	void skipFrames(EmuSystemTaskContext, int frames, EmuAudio*);
	bool skipForwardFrames(EmuSystemTaskContext, int frames);
	static void throwFileReadError();
	static void throwFileWriteError();
	static void throwMissingContentDirError();
    //region 爱吾修改
    void setCheatListAiWu(const std::list<std::string>& cheats);
	std::string getSystemSettingValueAiWu(std::string_view key);
	void setSystemSettingValueAiWu(std::string_view key, std::string_view value);
    //endregion

protected:
	ApplicationContext appCtx{};
public:
	EmuTiming timing;
protected:
	double audioFramesPerVideoFrameFloat{};
	double currentAudioFramesPerVideoFrame{};
	int audioFramesPerVideoFrame{};
	State state{};
	bool sessionOptionsSet{};
	BackupMemoryDirtyFlags backupMemoryDirtyFlags{};
	int8_t backupMemoryCounter{};
	FS::PathString contentDirectory_; // full directory path of content on disk, if any
	FS::PathString contentLocation_; // full path or URI to content
	FS::FileString contentFileName_; // name + extension of content, inside archive if any
	FS::FileString contentName_; // name of content from the original location without extension
	std::string contentDisplayName_; // more descriptive content name set by system
	FS::PathString contentSaveDirectory_;
	FS::PathString userSaveDirectory_;

	void setupContentUriPaths(CStringView uri, std::string_view displayName);
	void setupContentFilePaths(CStringView filePath, std::string_view displayName);
	void updateContentSaveDirectory();
	void closeAndSetupNew(CStringView path, std::string_view displayName);

public:
	double frameRateMultiplier{1.};
};

// Global instance access if required by the emulated system, valid if EmuApp::needsGlobalInstance initialized to true
EmuSystem &gSystem();

}
