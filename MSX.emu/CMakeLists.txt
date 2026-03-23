cmake_minimum_required(VERSION 4.1)

project(
	MSX.emu
	DESCRIPTION "MSX emulator"
	HOMEPAGE_URL "https://www.explusalpha.com/"
)

printConfigInfo()
configureAppTarget(msxemu)
addPkgConfigDepMultiConfig(msxemu emuframework)
addCxxModules(msxemu emuex imagine)
evalPkgConfigFlags(msxemu all)
target_include_directories(msxemu PRIVATE
	src/blueMSX
	src/blueMSX/SoundChips
	src/blueMSX/VideoChips
	src/blueMSX/Z80
	src/blueMSX/Media
	src/blueMSX/Common
	src/blueMSX/TinyXML
	src/blueMSX/VideoRender
	src/blueMSX/Board
	src/blueMSX/Arch
	src/blueMSX/Memory
	src/blueMSX/Emulator
	src/blueMSX/Input
	src/blueMSX/Utils
	src/blueMSX/Language
	src/blueMSX/IoDevice
	src/blueMSX/Debugger
)
target_compile_definitions(msxemu PRIVATE LSB_FIRST NO_ASM NO_EMBEDDED_SAMPLES)
# TODO: fix pointer type casts
target_compile_options(msxemu PRIVATE
	$<$<COMPILE_LANGUAGE:C>:-Werror=implicit-function-declaration>
	$<$<COMPILE_LANGUAGE:C>:-Wno-incompatible-pointer-types>
	-Wno-sign-compare
	-Wno-switch
	-Wno-implicit-fallthrough
	-Wno-unused-parameter
	-Wno-unused-function
	-Wno-unused-variable
)
if(CMAKE_C_COMPILER_ID MATCHES "GNU")
	target_compile_options(msxemu PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wno-global-module>)
endif()
add_subdirectory(src)