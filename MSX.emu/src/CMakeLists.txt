target_sources(msxemu PRIVATE FILE_SET CXX_MODULES
	BASE_DIRS main
	FILES main/system.ccm
)

target_sources(msxemu PRIVATE
	main/Main.cc
	main/AppMeta.cc
	main/options.cc
	main/input.cc
	main/EmuMenuViews.cc
	main/BlueMSXApi.cc
	main/Board.cc
	main/ziphelper.cc
	main/RomLoader.cc
	main/FrameBuffer.cc
	# Sound Chips
	blueMSX/SoundChips/AudioMixer.c
	blueMSX/SoundChips/YM2413.cpp
	blueMSX/SoundChips/SCC.c
	blueMSX/SoundChips/MsxPsg.c
	blueMSX/SoundChips/KeyClick.c
	blueMSX/SoundChips/DAC.c
	blueMSX/SoundChips/AY8910.c
	blueMSX/SoundChips/OpenMsxYM2413_2.cpp
	blueMSX/SoundChips/Y8950.c
	blueMSX/SoundChips/Fmopl.c
	blueMSX/SoundChips/Ymdeltat.c
	blueMSX/SoundChips/Moonsound.cpp
	blueMSX/SoundChips/OpenMsxYMF262.cpp
	blueMSX/SoundChips/OpenMsxYMF278.cpp
	blueMSX/SoundChips/VLM5030.c
	blueMSX/SoundChips/ym2151.c
	blueMSX/SoundChips/MameVLM5030.c
	blueMSX/SoundChips/MameYM2151.c
	# Coleco/SG1000 Sound Chips
	blueMSX/SoundChips/SN76489.c
	# Video Chips
	blueMSX/VideoChips/VideoManager.c
	blueMSX/VideoChips/VDP.c
	blueMSX/VideoChips/V9938.c
	blueMSX/VideoChips/CRTC6845.c
	# Io Device
	blueMSX/IoDevice/Casette.c
	blueMSX/IoDevice/Led.c
	blueMSX/IoDevice/Disk.c
	blueMSX/IoDevice/ScsiDevice.c
	blueMSX/IoDevice/Switches.c
	blueMSX/IoDevice/I8255.c
	blueMSX/IoDevice/SunriseIDE.c
	blueMSX/IoDevice/RTC.c
	blueMSX/IoDevice/MsxPPI.c
	blueMSX/IoDevice/WD2793.c
	blueMSX/IoDevice/FdcAudio.c
	blueMSX/IoDevice/MidiIO.c
	blueMSX/IoDevice/TC8566AF.c
	blueMSX/IoDevice/PrinterIO.c
	blueMSX/IoDevice/TurboRIO.c
	blueMSX/IoDevice/MSXMidi.c
	blueMSX/IoDevice/I8251.c
	blueMSX/IoDevice/I8254.c
	blueMSX/IoDevice/Z8530.c
	blueMSX/IoDevice/HarddiskIDE.c
	blueMSX/IoDevice/DirAsDisk.c
	# Utils
	blueMSX/Utils/SaveState.c
	blueMSX/Utils/TokenExtract.c
	blueMSX/Utils/IniFileParser.c
	blueMSX/Utils/IsFileExtension.c
	# Input
	blueMSX/Input/JoystickPort.c
	blueMSX/Input/InputEvent.c
	blueMSX/Input/CoinDevice.c
	blueMSX/Input/MsxJoystick.c
	blueMSX/Input/MsxMouse.c
	# Coleco Input
	blueMSX/Input/ColecoJoystick.c
	blueMSX/Input/ColecoSteeringWheel.c
	blueMSX/Input/ColecoSuperAction.c
	# Board
	blueMSX/Board/Machine.c
	blueMSX/Board/MSX.c
	blueMSX/Board/Coleco.c
	# Memory
	blueMSX/Memory/ram1kBMirrored.c
	blueMSX/Memory/ramNormal.c
	blueMSX/Debugger/DebugDeviceManager.c
	blueMSX/Memory/DeviceManager.c
	blueMSX/Memory/SlotManager.c
	blueMSX/Memory/ramMapper.c
	blueMSX/Memory/ramMapperIo.c
	blueMSX/Memory/IoPort.c
	blueMSX/Memory/romMapperDRAM.c
	blueMSX/Memory/MegaromCartridge.c
	blueMSX/Memory/romMapperFMPAC.c
	blueMSX/Memory/sramLoader.c
	blueMSX/Memory/romMapperPAC.c
	blueMSX/Memory/AmdFlash.c
	blueMSX/Memory/romMapperMegaFlashRomScc.c
	blueMSX/Memory/romMapperSCCplus.c
	blueMSX/Memory/romMapperNormal.c
	blueMSX/Memory/romMapperPlain.c
	blueMSX/Memory/romMapperBasic.c
	blueMSX/Memory/romMapperMegaRAM.c
	blueMSX/Memory/romMapperStandard.c
	blueMSX/Memory/romMapperMsxDos2.c
	blueMSX/Memory/romMapperKonami5.c
	blueMSX/Memory/romMapperKonami4.c
	blueMSX/Memory/romMapperMsxMusic.c
	blueMSX/Memory/romMapperMsxAudio.c
	blueMSX/Memory/romMapperPhilipsFdc.c
	blueMSX/Memory/romMapperASCII8.c
	blueMSX/Memory/romMapperASCII16.c
	blueMSX/Memory/romMapperF4device.c
	blueMSX/Memory/romMapperKanji.c
	blueMSX/Memory/romMapperMsxPrn.c
	blueMSX/Memory/romMapperCasette.c
	blueMSX/Memory/romMapperTC8566AF.c
	blueMSX/Memory/romMapperMoonsound.c
	blueMSX/Memory/sramMapperS1985.c
	blueMSX/Memory/romMapperS1990.c
	blueMSX/Memory/romMapperTurboRTimer.c
	blueMSX/Memory/romMapperTurboRPcm.c
	blueMSX/Memory/romMapperNms1210Rs232.c
	blueMSX/Memory/romMapperSonyHBI55.c
	blueMSX/Memory/romMapperASCII8sram.c
	blueMSX/Memory/romMapperASCII16sram.c
	blueMSX/Memory/romMapperRType.c
	blueMSX/Memory/romMapperCrossBlaim.c
	blueMSX/Memory/romMapperHarryFox.c
	blueMSX/Memory/romMapperKorean80.c
	blueMSX/Memory/romMapperKorean126.c
	blueMSX/Memory/romMapperKonami4nf.c
	blueMSX/Memory/romMapperASCII16nf.c
	blueMSX/Memory/romMapperHolyQuran.c
	blueMSX/Memory/romMapperLodeRunner.c
	blueMSX/Memory/romMapperKorean90.c
	blueMSX/Memory/romMapperKonamiSynth.c
	blueMSX/Memory/romMapperMajutsushi.c
	blueMSX/Memory/romMapperDisk.c
	blueMSX/Memory/romMapperKanji12.c
	blueMSX/Memory/romMapperHalnote.c
	blueMSX/Memory/romMapperMicrosol.c
	blueMSX/Memory/romMapperCvMegaCart.c
	blueMSX/Memory/romMapperGameMaster2.c
	blueMSX/Memory/romMapperPlayBall.c
	blueMSX/Memory/romMapperKonamiWordPro.c
	blueMSX/Memory/romMapperKonamiKeyboardMaster.c
	blueMSX/Memory/romMapperForteII.c
	blueMSX/Memory/romMapperKoei.c
	blueMSX/Memory/romMapperArc.c
	blueMSX/Memory/romMapperMicrosolVmx80.c
	blueMSX/Memory/romMapperSonyHBIV1.c
	blueMSX/Memory/romMapperNationalFdc.c
	blueMSX/Memory/romMapperMatraINK.c
	blueMSX/Memory/romMapperFMPAK.c
	blueMSX/Memory/romMapperNettouYakyuu.c
	blueMSX/Memory/romMapperFmDas.c
	blueMSX/Memory/romMapperSfg05.c
	blueMSX/Memory/romMapperGIDE.c
	blueMSX/Memory/sramMapperMatsuchita.c
	blueMSX/Memory/romMapperNational.c
	blueMSX/Memory/romMapperBunsetu.c
	blueMSX/Memory/romMapperSunriseIDE.c
	blueMSX/Memory/romMapperNet.c
	# Z80
	blueMSX/Z80/R800.c
	blueMSX/Z80/R800SaveState.c
	# Others 
	blueMSX/Bios/Patch.c
	blueMSX/Language/LanguageMinimal.c
)

if(useXMLDB)
	target_sources(msxemu PRIVATE
		# TinyXML
		blueMSX/TinyXML/tinystr.cpp
		blueMSX/TinyXML/tinyxml.cpp
		blueMSX/TinyXML/tinyxmlerror.cpp
		blueMSX/TinyXML/tinyxmlparser.cpp
		# Media
		blueMSX/Media/MediaDb.cpp
		blueMSX/Media/Sha1.cpp
		blueMSX/Media/Crc32Calc.c
	)
else()
	target_sources(msxemu PRIVATE main/MediaDb.cc)
endif()