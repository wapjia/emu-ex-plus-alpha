target_sources(pceemu PRIVATE FILE_SET CXX_MODULES
	BASE_DIRS main
	FILES main/system.ccm
)

target_sources(pceemu PRIVATE
	main/Main.cc
	main/AppMeta.cc
	main/options.cc
	main/input.cc
	main/EmuMenuViews.cc
	pce_fast/input.cpp
	pce_fast/vdc.cpp
	pce_fast/huc6280.cpp
	pce_fast/pce.cpp
	pce_fast/huc.cpp
	pce_fast/pcecd.cpp
	pce_fast/pcecd_drive.cpp
	pce_fast/psg.cpp
	pce/huc.cpp
	pce/huc6280.cpp
	pce/input.cpp
	pce/mcgenjin.cpp
	pce/pce.cpp
	pce/pcecd.cpp
	pce/tsushin.cpp
	pce/vce.cpp
	pce/input/gamepad.cpp
	pce/input/mouse.cpp
	pce/input/tsushinkb.cpp
)