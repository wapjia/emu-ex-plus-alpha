cmake_minimum_required(VERSION 4.1)

project(
	C64.emu
	DESCRIPTION "Commodore 64 emulator"
	HOMEPAGE_URL "https://www.explusalpha.com/"
)

printConfigInfo()
configureAppTarget(c64emu)
addPkgConfigDepMultiConfig(c64emu emuframework)
addCxxModules(c64emu emuex imagine)
evalPkgConfigFlags(c64emu all)

set(exportedSymbols
	archdep_default_rtc_file_name
	archdep_file_size
	kbd_arch_keyname_to_keynum
	sound_init_dummy_device
	sysfile_init
	sysfile_load
	sysfile_locate
	sysfile_open
	ui_display_tape_counter
	video_arch_canvas_init
	video_canvas_create
	video_canvas_destroy
	video_canvas_refresh
	video_canvas_resize
	video_canvas_set_palette
	vsyncarch_refresh_frequency_changed
	vsync_do_vsync2
	zfile_fopen
)

setExportedSymbols(c64emu release ${exportedSymbols})

setExportedSymbols(c64emu debug
	${exportedSymbols}
	log_message
	log_warning
	log_error
	log_debug
	log_verbose
	log_printf
	archdep_startup_log_error
	ui_error
	uimon_out
	uimon_petscii_out
	uimon_petscii_upper_out
	uimon_scrcode_out
	uimon_scrcode_upper_out
)

add_library(c64emuInterface INTERFACE)

target_include_directories(c64emuInterface INTERFACE
	src/config
	src/vice
	src/vice/c64
	src/vice/c64/cart
	src/vice/c128
	src/vice/cbm2
	src/vice/cbm2/cart
	src/vice/pet
	src/vice/plus4
	src/vice/plus4/cart
	src/vice/drive
	src/vice/lib/p64
	src/vice/sid
	src/vice/tape
	src/vice/userport
	src/vice/video
	src/vice/drive/iec/c64exp
	src/vice/core
	src/vice/core/rtc
	src/vice/vdrive
	src/vice/imagecontents
	src/vice/monitor
	src/vice/platform
	src/vice/printerdrv
	src/vice/raster
	src/vice/c64dtv
	src/vice/vicii
	src/vice/viciisc
	src/vice/vdc
	src/vice/vic20
	src/vice/vic20/cart
	src/vice/crtc
	src/vice/tapeport
	src/vice/joyport
	src/vice/samplerdrv
	src/vice/drive/iec
	src/vice/drive/ieee
	src/vice/drive/tcbm
	src/vice/diag
	src/vice/rs232drv
	src/vice/datasette
	src/vice/fsdevice
	src/vice/arch/shared
	src/vice/arch/shared/hotkeys
)

target_compile_definitions(c64emuInterface INTERFACE
	STDC_HEADERS=1
	HAVE_SYS_TYPES_H=1
	HAVE_SYS_STAT_H=1
	HAVE_STDLIB_H=1
	HAVE_STRING_H=1
	HAVE_MEMORY_H=1
	HAVE_STRINGS_H=1
	HAVE_INTTYPES_H=1
	HAVE_STDINT_H=1
	HAVE_UNISTD_H=1
)

target_compile_options(c64emuInterface INTERFACE
	$<$<COMPILE_LANGUAGE:C>:-Werror=implicit-function-declaration>
	-Wno-implicit-fallthrough
	-Wno-sign-compare
	-Wno-unused-parameter
	-Wno-unused-function
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU")
	target_compile_options(c64emuInterface INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wno-global-module>)
endif()

target_link_libraries(c64emu PRIVATE c64emuInterface)
add_subdirectory(src)