cmake_minimum_required(VERSION 4.1)

project(
	GBA.emu
	DESCRIPTION "Gameboy Advance emulator"
	HOMEPAGE_URL "https://www.explusalpha.com/"
)

printConfigInfo()
configureAppTarget(gbaemu)
addPkgConfigDepMultiConfig(gbaemu emuframework)
addCxxModules(gbaemu emuex imagine)
evalPkgConfigFlags(gbaemu all)
target_compile_definitions(gbaemu PRIVATE HAVE_ZLIB_H FINAL_VERSION C_CORE NO_PNG NO_LINK NO_DEBUGGER)
target_compile_options(gbaemu PRIVATE -Wno-unused-parameter -Wno-unused-function -Wno-unused-but-set-variable)
add_subdirectory(src)