set(Z80_PATH "${EMUFRAMEWORK_PATH}/../MD.emu/src/genplus-gx/z80")
target_include_directories(neoemu PRIVATE "${Z80_PATH}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	# needed for Z80CPU::makeFlagTables()
	target_compile_options(neoemu PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fconstexpr-steps=10000000>)
endif()

target_sources(neoemu PRIVATE FILE_SET CXX_MODULES
	BASE_DIRS main
	FILES main/system.ccm
)

target_sources(neoemu PRIVATE
	main/Main.cc
	main/AppMeta.cc
	main/input.cc
	main/options.cc
	main/state.cc
	main/unzip.cc
	main/EmuMenuViews.cc
	${Z80_PATH}/z80.cc
	gngeo/mamez80_interf.cc
	gngeo/ym2610/2610intf.c
	gngeo/ym2610/ym2610.c
	gngeo/debug.c
	gngeo/emu.c
	gngeo/fileio.c
	gngeo/mame_layer.c
	gngeo/memory.c
	gngeo/neoboot.c
	gngeo/neocrypt.c
	gngeo/pd4990a.c
	gngeo/roms.c
	gngeo/timer.c
	gngeo/video.c
)

if(ARCH STREQUAL arm)
	if(ENV STREQUAL ios)
		target_sources(neoemu PRIVATE gngeo/cyclone_interf.c gngeo/cyclone/Cyclone-apple.s)
		set_source_files_properties(gngeo/cyclone/Cyclone-apple.s PROPERTIES POSITION_INDEPENDENT_CODE OFF)
	else()
		target_sources(neoemu PRIVATE gngeo/video_arm.s gngeo/cyclone_interf.c gngeo/cyclone/Cyclone.s)
		set_source_files_properties(gngeo/cyclone/Cyclone.s PROPERTIES POSITION_INDEPENDENT_CODE OFF)
		set_source_files_properties(gngeo/cyclone/video_arm.s PROPERTIES POSITION_INDEPENDENT_CODE OFF)
	endif()
else()
	set(M68K_PATH "${EMUFRAMEWORK_PATH}/../MD.emu/src/genplus-gx/m68k")
	target_include_directories(neoemu PRIVATE "${M68K_PATH}")
	target_sources(neoemu PRIVATE "${M68K_PATH}/musashi/m68kcpu.cc" gngeo/musashi_interf.cc)
endif()