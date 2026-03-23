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

#include <emuframework/defs.hh>
#include <emuframework/VController.hh>
#include <emuframework/TurboInput.hh>
#include <emuframework/ToggleInput.hh>
#include <emuframework/inputDefs.hh>
#include <emuframework/AppKeyCode.hh>
#include <emuframework/EmuInput.hh>
#ifdef IG_USE_MODULES
import imagine;
import std;
#else
#include <imagine/input/inputDefs.hh>
#include <string>
#include <string_view>
#include <memory>
#include <algorithm>
#endif

namespace EmuEx
{

inline constexpr KeyCategory appKeyCategory{"In-Emulation Actions", appKeys};

class InputManager
{
public:
	VController vController;
	std::vector<std::unique_ptr<KeyConfig>> customKeyConfigs;
	std::vector<std::unique_ptr<InputDeviceSavedConfig>> savedDevConfigs;
	std::vector<std::unique_ptr<InputDeviceSavedSessionConfig>> savedSessionDevConfigs;
	TurboInput turboActions;
	ToggleInput toggleInput;
	DelegateFunc<void ()> onUpdateDevices;
	bool turboModifierActive{};

	InputManager(ApplicationContext ctx):
		vController{ctx} {}
	bool handleKeyInput(EmuApp&, KeyInfo, const Input::Event& srcEvent);
	bool handleAppActionKeyInput(EmuApp&, InputAction, const Input::Event& srcEvent);
	void handleSystemKeyInput(EmuApp&, KeyInfo, Input::Action, uint32_t metaState = 0, SystemKeyInputFlags = {});
	void updateInputDevices(ApplicationContext);
	KeyConfig* customKeyConfig(std::string_view name, const Input::Device&) const;
	KeyConfigDesc keyConfig(std::string_view name, const Input::Device&) const;
	void deleteKeyProfile(ApplicationContext, KeyConfig*);
	void deleteDeviceSavedConfig(ApplicationContext, const InputDeviceSavedConfig&);
	void deleteDeviceSavedConfig(ApplicationContext, const InputDeviceSavedSessionConfig&);
	void resetSessionOptions(ApplicationContext);
	bool readSessionConfig(ApplicationContext, MapIO&, uint16_t);
	void writeSessionConfig(FileIO&) const;
	bool readInputDeviceSessionConfigs(ApplicationContext, MapIO&);
	void writeInputDeviceSessionConfigs(FileIO&) const;
	void writeCustomKeyConfigs(FileIO&) const;
	void writeSavedInputDevices(ApplicationContext, FileIO&) const;
	bool readCustomKeyConfig(MapIO&);
	bool readSavedInputDevices(MapIO&);
	KeyConfigDesc defaultConfig(const Input::Device&) const;
	KeyInfo transpose(KeyInfo, int index) const;
	std::string toString(KeyInfo) const;
	std::string toString(KeyCode, KeyFlags) const;
	const KeyCategory* categoryOfKeyCode(KeyInfo) const;
	KeyInfo validateSystemKey(KeyInfo, bool isUIKey) const;
	void updateKeyboardMapping();
	void toggleKeyboard();

private:
	static constexpr SystemLogger log{"InputManager"};
};

}
