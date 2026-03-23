cmake_minimum_required(VERSION 4.1)

project(
	PCE.emu
	DESCRIPTION "TurboGrafx 16/PC-Engine emulator"
	HOMEPAGE_URL "https://www.explusalpha.com/"
)

include(emuFrameworkUtils.cmake)
printConfigInfo()
configureAppTarget(pceemu)
addMednafenFlags(pceemu cd)
addPkgConfigDepMultiConfig(pceemu emuframework)
addCxxModules(pceemu emuex imagine)
evalPkgConfigFlags(pceemu all)
add_subdirectory(src)