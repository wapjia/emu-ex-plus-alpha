cmake_minimum_required(VERSION 4.1)

project(
	Saturn.emu
	DESCRIPTION "Sega Saturn emulator"
	HOMEPAGE_URL "https://www.explusalpha.com/"
)

include(emuFrameworkUtils.cmake)
printConfigInfo()
configureAppTarget(saturnemu)
addMednafenFlags(saturnemu cd)
addPkgConfigDepMultiConfig(saturnemu emuframework)
addCxxModules(saturnemu emuex imagine)
evalPkgConfigFlags(saturnemu all)
add_subdirectory(src)
set_source_files_properties(src/ss/scu_dsp_gen.cpp PROPERTIES COMPILE_OPTIONS "-fno-sanitize=address,undefined")
set_source_files_properties(src/ss/vdp2_render.cpp PROPERTIES COMPILE_OPTIONS "-fno-sanitize=address,undefined")
set_source_files_properties(src/ss/ss.cpp PROPERTIES COMPILE_OPTIONS "-fno-sanitize=address,undefined")