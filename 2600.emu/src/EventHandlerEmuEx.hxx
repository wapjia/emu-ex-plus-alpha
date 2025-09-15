#pragma once

// Customized minimal EventHandler class needed for 2600.emu

#include <stella/emucore/Event.hxx>
#include <stella/emucore/EventHandlerConstants.hxx>
#include <stella/emucore/Control.hxx>
#include <cmath>

class OSystem;
class Properties;

class EventHandler
{
public:
	// Enumeration representing the different states of operation
	enum State {
		S_NONE,
		S_EMULATE,
		S_PAUSE,
		S_LAUNCHER,
		S_MENU,
		S_CMDMENU,
		S_DEBUGGER
	};

	EventHandler(OSystem& osystem) {}
	Event& event() { return myEvent; }
	const Event& event() const { return myEvent; }
	void handleEvent(Event::Type event, Int32 value = 1, bool repeated = false) {}
	void allowAllDirections(bool allow) {}
	EventHandlerState state() const { return EventHandlerState::EMULATION; }
	bool inTIAMode() const { return true; }
	void enableEmulationKeyMappings() {}
	void enableEmulationJoyMappings() {}
	void setMouseControllerMode(string_view enable) {}
	void set7800Mode() {}
	void defineKeyControllerMappings(const Controller::Type type, Controller::Jack port,
		const Properties& properties,
		Controller::Type qtType1 = Controller::Type::Unknown,
		Controller::Type qtType2 = Controller::Type::Unknown)  {}
	void defineJoyControllerMappings(const Controller::Type type, Controller::Jack port,
		const Properties& properties,
		Controller::Type qtType1 = Controller::Type::Unknown,
		Controller::Type qtType2 = Controller::Type::Unknown) {}

private:
	Event myEvent{};
};
