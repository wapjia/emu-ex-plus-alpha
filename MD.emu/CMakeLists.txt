cmake_minimum_required(VERSION 4.1)

project(
	MD.emu
	DESCRIPTION "Sega Genesis/Mega Drive emulator"
	HOMEPAGE_URL "https://www.explusalpha.com/"
)

include(emuFrameworkUtils.cmake)
printConfigInfo()
configureAppTarget(mdemu)
addMednafenFlags(mdemu cd)
addPkgConfigDepMultiConfig(mdemu emuframework)
addCxxModules(mdemu emuex imagine)
evalPkgConfigFlags(mdemu all)
target_include_directories(mdemu PRIVATE
	src/genplus-gx
	src/genplus-gx/m68k
	src/genplus-gx/z80
	src/genplus-gx/input_hw
	src/genplus-gx/sound
	src/genplus-gx/cart_hw
	src/genplus-gx/cart_hw/svp
)
target_compile_definitions(mdemu PRIVATE LSB_FIRST NO_SYSTEM_PICO MDFN_CD_NO_CCD)
target_compile_options(mdemu PRIVATE -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-function)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	# needed for Z80CPU::makeFlagTables()
	target_compile_options(mdemu PRIVATE -fconstexpr-steps=10000000)
endif()
add_subdirectory(src)