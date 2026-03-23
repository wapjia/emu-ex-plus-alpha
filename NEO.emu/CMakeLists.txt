cmake_minimum_required(VERSION 4.1)

project(
	NEO.emu
	DESCRIPTION "Neo Geo emulator"
	HOMEPAGE_URL "https://www.explusalpha.com/"
)

if(ARCH STREQUAL arm)
	enable_language(ASM)
endif()

include(emuFrameworkUtils.cmake)
printConfigInfo()
configureAppTarget(neoemu)
addPkgConfigDepMultiConfig(neoemu emuframework)
addCxxModules(neoemu emuex imagine)
evalPkgConfigFlags(neoemu all)
target_compile_definitions(neoemu PRIVATE
	$<$<COMPILE_LANGUAGE:C,CXX>:HAVE_CONFIG_H LSB_FIRST>
)
target_compile_options(neoemu PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-sign-compare -Wno-unused-parameter -Wno-unused-function>)
if(CMAKE_C_COMPILER_ID MATCHES "GNU")
	target_compile_options(neoemu PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wno-global-module>)
endif()
add_subdirectory(src)
