cmake_minimum_required(VERSION 4.1)

project(
	GBC.emu
	DESCRIPTION "Gameboy Color emulator"
	HOMEPAGE_URL "https://www.explusalpha.com/"
)

printConfigInfo()
configureAppTarget(gbcemu)
addPkgConfigDepMultiConfig(gbcemu emuframework)
addCxxModules(gbcemu emuex imagine)
evalPkgConfigFlags(gbcemu all)
target_include_directories(gbcemu PRIVATE
	src/common
	src/libgambatte/include
	src/libgambatte/src
)
target_compile_definitions(gbcemu PRIVATE HAVE_STDINT_H GAMBATTE_NO_OSD)
target_compile_options(gbcemu PRIVATE -Wno-deprecated-anon-enum-enum-conversion -Wno-deprecated-enum-enum-conversion -Wno-deprecated-enum-float-conversion)
add_subdirectory(src)