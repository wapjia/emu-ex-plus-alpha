#pragma once

// Customized minimal MediaFactory class needed for 2600.emu

#include "bspf.hxx"

#include "Settings.hxx"
#include "SerialPort.hxx"

#include "EventHandlerEmuEx.hxx"
#include "FrameBufferEmuEx.hxx"
#include "SoundEmuEx.hh"

class AudioSettings;
class OSystem;

class MediaFactory
{
public:
	static unique_ptr<Settings> createSettings() { return make_unique<Settings>(); }
	static unique_ptr<SerialPort> createSerialPort() { return make_unique<SerialPort>(); }
	static unique_ptr<Sound> createAudio(OSystem& osystem, AudioSettings& audioSettings) { return make_unique<SoundEmuEx>(osystem); }
	static unique_ptr<EventHandler> createEventHandler(OSystem& osystem) { return make_unique<EventHandler>(osystem); }
	static void cleanUp() {}
	static string backendName() { return "Custom backend"; }
	static bool supportsURL() { return false; }
	static bool openURL(string_view url) { return false; }

private:
	// Following constructors and assignment operators not supported
	MediaFactory() = delete;
	~MediaFactory() = delete;
	MediaFactory(const MediaFactory&) = delete;
	MediaFactory(MediaFactory&&) = delete;
	MediaFactory& operator=(const MediaFactory&) = delete;
	MediaFactory& operator=(MediaFactory&&) = delete;
};
