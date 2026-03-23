target_sources(emuframework PRIVATE FILE_SET CXX_MODULES
	BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
	FILES configFile.ccm pathUtils.ccm
)

install(FILES emuex.ccm DESTINATION share/modules)

target_sources(
	emuframework PRIVATE
	AppMeta.cc
	AssetManager.cc
	AutosaveManager.cc
	ConfigFile.cc
	EmuApp.cc
	EmuAudio.cc
	EmuInput.cc
	EmuOptions.cc
	EmuSystem.cc
	EmuSystemTask.cc
	EmuTiming.cc
	EmuVideo.cc
	EmuVideoLayer.cc
	InputDeviceConfig.cc
	InputDeviceData.cc
	KeyConfig.cc
	OutputTimingManager.cc
	RecentContent.cc
	RewindManager.cc
	ToggleInput.cc
	TurboInput.cc
	VideoImageEffect.cc
	VideoImageOverlay.cc
	gui/AudioOptionView.cc
	gui/AutosaveSlotView.cc
	gui/BundledGamesView.cc
	gui/ButtonConfigView.cc
	gui/CPUAffinityView.cc
	gui/CreditsView.cc
	gui/EmuInputView.cc
	gui/EmuView.cc
	gui/EmuViewController.cc
	gui/FilePathOptionView.cc
	gui/FilePicker.cc
	gui/FrameTimingView.cc
	gui/GUIOptionView.cc
	gui/InputManagerView.cc
	gui/InputOverridesView.cc
	gui/LoadProgressView.cc
	gui/MainMenuView.cc
	gui/PlaceVControlsView.cc
	gui/PlaceVideoView.cc
	gui/RecentContentView.cc
	gui/StateSlotView.cc
	gui/SystemActionsView.cc
	gui/SystemOptionView.cc
	gui/TouchConfigView.cc
	gui/VideoOptionView.cc
	vcontrols/VController.cc
	vcontrols/VControllerButton.cc
	vcontrols/VControllerButtonGroup.cc
	vcontrols/VControllerDPad.cc
	vcontrols/VControllerKeyboard.cc
)
