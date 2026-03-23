cmake_minimum_required(VERSION 4.1)

project(
	2600.emu
	DESCRIPTION "Atari 2600 emulator"
	HOMEPAGE_URL "https://www.explusalpha.com/"
)

printConfigInfo()
configureAppTarget(2600emu)
addPkgConfigDepMultiConfig(2600emu emuframework)
addCxxModules(2600emu emuex imagine)
evalPkgConfigFlags(2600emu all)
target_include_directories(2600emu PRIVATE
	src/stella/emucore
	src/stella/emucore/tia
	src/stella/emucore/elf
	src/stella/common
	src/stella/common/tv_filters
	src/stella/gui
)
target_compile_definitions(2600emu PRIVATE EMU_EX_PLATFORM SOUND_SUPPORT THUMB_SUPPORT)
target_compile_options(2600emu PRIVATE -Wno-unused-parameter -Wno-deprecated-literal-operator)
add_subdirectory(src)