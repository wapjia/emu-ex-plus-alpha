#pragma once

// Customized minimal OSystem class needed for 2600.emu

class Cartridge;
class CheatManager;
class CommandMenu;
class Debugger;
class Launcher;
class Menu;
class Properties;
class Sound;
class VideoDialog;

#include <stella/common/bspf.hxx>
#include <stella/common/StateManager.hxx>
#include <stella/common/AudioSettings.hxx>
#include <stella/emucore/PropsSet.hxx>
#include <stella/emucore/Console.hxx>
#include <stella/emucore/FrameBufferConstants.hxx>
#include <stella/emucore/EventHandlerConstants.hxx>
#include <stella/emucore/Settings.hxx>
#include <stella/emucore/Random.hxx>
#include <FSNode.hxx>
#include <SoundEmuEx.hh>
#include <EventHandler.hxx>
#include <FrameBuffer.hxx>
#include <optional>

namespace EmuEx
{
class EmuAudio;
class EmuApp;
}

class OSystem
{
	friend class EventHandler;

public:
	OSystem(EmuEx::EmuApp &);
	auto& eventHandler(this auto&& self) { return self.myEventHandler; }
	auto& random(this auto&& self) { return self.myRandom; }
	auto& frameBuffer(this auto&& self) { return self.myFrameBuffer; }
	auto& sound(this auto&& self) { return self.mySound; }
	auto& settings(this auto&& self) { return self.mySettings; }
	auto& propSet(this auto&& self) { return self.myPropSet; }
	auto& state(this auto&& self) { return self.myStateManager; }
	auto& soundEmuEx(this auto&& self) { return self.mySound; }
	auto& console(this auto&& self) { return *self.myConsole; }
	bool hasConsole() const { return (bool)myConsole; }
	void makeConsole(unique_ptr<Cartridge>& cart, const Properties& props, const char* gamePath);
	void deleteConsole();
	void setSoundMixRate(int mixRate);

	#ifdef DEBUGGER_SUPPORT
	void createDebugger(Console& console);
	Debugger& debugger() const { return *myDebugger; }
	#endif

	#ifdef CHEATCODE_SUPPORT
	CheatManager& cheat() const { return *myCheatManager; }
	#endif

	FSNode stateDir() const;
	FSNode nvramDir(std::string_view name) const;
	FSNode baseDir(std::string_view name) const;

	bool checkUserPalette(bool outputError = false) const { return false; }
	FSNode paletteFile() const { return FSNode{""}; }

	const FSNode& romFile() const { return myRomFile; };

	void resetFps() {}

	auto& app(this auto&& self) { return *self.appPtr; }

protected:
	EmuEx::EmuApp* appPtr{};
	std::optional<Console> myConsole{};
	Settings mySettings{};
	AudioSettings myAudioSettings{mySettings};
	Random myRandom;
	FrameBuffer myFrameBuffer{*this};
	EventHandler myEventHandler{*this};
	PropertiesSet myPropSet{};
	StateManager myStateManager{*this};
	SoundEmuEx mySound{*this};
	FSNode myRomFile{};
};
