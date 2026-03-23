target_sources(swanemu PRIVATE FILE_SET CXX_MODULES
	BASE_DIRS main
	FILES main/system.ccm
)

target_sources(swanemu PRIVATE
	main/Main.cc
	main/AppMeta.cc
	main/options.cc
	main/input.cc
	main/EmuMenuViews.cc
	wswan/comm.cpp
	wswan/gfx.cpp
	wswan/memory.cpp
	wswan/tcache.cpp
	wswan/interrupt.cpp
	wswan/rtc.cpp
	wswan/v30mz.cpp
	wswan/eeprom.cpp
	wswan/main.cpp
	wswan/sound.cpp
)