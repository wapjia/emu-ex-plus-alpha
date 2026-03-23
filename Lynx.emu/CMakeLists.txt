cmake_minimum_required(VERSION 4.1)

project(
	Lynx.emu
	DESCRIPTION "Lynx emulator"
	HOMEPAGE_URL "https://www.explusalpha.com/"
)

include(emuFrameworkUtils.cmake)
printConfigInfo()
configureAppTarget(lynxemu)
addMednafenFlags(lynxemu)
addPkgConfigDepMultiConfig(lynxemu emuframework)
addCxxModules(lynxemu emuex imagine)
evalPkgConfigFlags(lynxemu all)
add_subdirectory(src)