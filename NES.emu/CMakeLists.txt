cmake_minimum_required(VERSION 4.1)

project(
	NES.emu
	DESCRIPTION "NES/Famicom emulator"
	HOMEPAGE_URL "https://www.explusalpha.com/"
)

printConfigInfo()
configureAppTarget(nesemu)
addPkgConfigDepMultiConfig(nesemu emuframework)
addCxxModules(nesemu emuex imagine)
evalPkgConfigFlags(nesemu all)
target_include_directories(nesemu PRIVATE src/fceu)
target_compile_definitions(nesemu PRIVATE HAVE_ASPRINTF PSS_STYLE=1 LSB_FIRST FRAMESKIP)
# TODO: fix pointer type casts
target_compile_options(nesemu PRIVATE
	-Wno-register
	-Wno-sign-compare
	-Wno-missing-field-initializers
	-Wno-switch
	-Wno-bitwise-op-parentheses
	-Wno-expansion-to-defined
	-Wno-unused-local-typedefs
	-Wno-unused-parameter
	-Wno-deprecated-volatile
	-Wno-non-c-typedef-for-linkage
) 
add_subdirectory(src)