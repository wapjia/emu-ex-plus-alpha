file(REAL_PATH ${CMAKE_CURRENT_LIST_FILE} EMUFRAMEWORK_PATH)
cmake_path(GET EMUFRAMEWORK_PATH PARENT_PATH EMUFRAMEWORK_PATH)
message("EmuFramework Path: ${EMUFRAMEWORK_PATH}")

function(addMednafenFlags target)
	target_include_directories(${target} PRIVATE
		${EMUFRAMEWORK_PATH}/include/shared
		${EMUFRAMEWORK_PATH}/include/shared/mednafen
		${EMUFRAMEWORK_PATH}/include/shared/lzma
		${EMUFRAMEWORK_PATH}/src/shared
		${EMUFRAMEWORK_PATH}/src/shared/mednafen/hw_misc
		${EMUFRAMEWORK_PATH}/src/shared/mednafen/hw_sound
	)
	target_link_libraries(${target} PRIVATE mednafen_common${GEN_TARGET_EXT})
	target_compile_definitions(${target} PRIVATE HAVE_CONFIG_H)
	target_compile_options(${target} PRIVATE
		-Wno-missing-field-initializers
		-Wno-unused-parameter
		-Wno-unused-function
		-Wno-implicit-fallthrough
		-Wno-deprecated-enum-enum-conversion
	)
	if(ARGV1 STREQUAL cd)
		addPkgLibvorbis(${target})
		addPkgFlac(${target})
	endif()
endfunction()