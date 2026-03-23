cmake_minimum_required(VERSION 4.1)

project(
	Snes9xEX
	DESCRIPTION "SNES/Super Famicom emulator"
	HOMEPAGE_URL "https://www.explusalpha.com/"
)

printConfigInfo()
configureAppTarget(s9x)
addPkgConfigDepMultiConfig(s9x emuframework)
addCxxModules(s9x emuex imagine)
evalPkgConfigFlags(s9x all)
target_include_directories(s9x PRIVATE src/snes9x-1.43)
target_compile_definitions(s9x PRIVATE
	HAVE_STRINGS_H
	VAR_CYCLES
	RIGHTSHIFT_IS_SAR
	USE_OPENGL
	CPU_SHUTDOWN
	SPC700_SHUTDOWN
	SPC700_C
	SDD1_DECOMP
	NOASM
	PIXEL_FORMAT=RGB565
	SNES9X_VERSION_1_4
)
target_compile_options(s9x PRIVATE -Wno-register -Wno-implicit-fallthrough)
add_subdirectory(src)