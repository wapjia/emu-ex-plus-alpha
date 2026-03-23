target_sources(c64emu PRIVATE FILE_SET CXX_MODULES
	BASE_DIRS main
	FILES main/system.ccm main/plugin.ccm
)

target_sources(c64emu PRIVATE
	main/Main.cc
	main/AppMeta.cc
	main/options.cc
	main/input.cc
	main/EmuMenuViews.cc
	main/VicePlugin.cc
	main/resources.cc
	main/sysfile.cc
	main/video.cc
	main/sound.cc
	main/zfile.cc
	$<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/main/log.cc>
)

function(addObjectLib target prefix)
	if(prefix)
		list(TRANSFORM ARGN PREPEND "${prefix}/")
	endif()
	#message("adding object lib:${target} sources:${ARGN}")
	add_library(${target} OBJECT ${ARGN})
	target_link_libraries(${target} PRIVATE c64emuInterface)
	set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endfunction()

function(addGlobObjectLib target)
	file(GLOB srcs RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" ${ARGN})
	addObjectLib(${target} "" ${srcs})
endfunction()

function(addObjects allObjsOut prefix)
	foreach(obj ${ARGN})
		addObjectLib(${obj} ${prefix} ${obj})
		list(APPEND allObjs $<TARGET_OBJECTS:${obj}>)
	endforeach()
	set(${allObjsOut} ${allObjs} PARENT_SCOPE)
endfunction()

addObjectLib(libbase "vice"
	alarm.c
	attach.c
	autostart.c
	autostart-prg.c
	charset.c
	clipboard.c
	cbmdos.c
	cbmimage.c
	crc32.c
	crt.c
	debug.c
	dma.c
	event.c
	fliplist.c
	gcr.c
	info.c
	init.c
	interrupt.c
	kbdbuf.c
	keyboard.c
	keymap.c
	lib.c
	machine-bus.c
	machine.c
	network.c
	opencbmlib.c
	palette.c
	profiler.c
	ram.c
	rawfile.c
	rawnet.c
	resources.c
	romset.c
	sha1.c
	snapshot.c
	socket.c
	sound.c
	traps.c
	util.c
	vsync.c
	zipcode.c
)

addObjectLib(libc64cartsystem "vice/c64/cart"
	c64cart.c
	c64carthooks.c
	c64cartmem.c
)

addObjectLib(libc64cart "vice/c64/cart"
	actionreplay2.c
	actionreplay3.c
	actionreplay4.c
	actionreplay.c
	atomicpower.c
	bisplus.c
	blackbox3.c
	blackbox4.c
	blackbox8.c
	blackbox9.c
	bmpdataturbo.c
	c64-generic.c
	c64-midi.c
	c64tpi.c
	capture.c
	clockport.c
	comal80.c
	cpmcart.c
	daa.c
	debugcart.c
	delaep256.c
	delaep64.c
	delaep7x8.c
	diashowmaker.c
	dinamic.c
	dqbb.c
	drean.c
	easycalc.c
	easyflash.c
	epyxfastload.c
	exos.c
	expert.c
	final.c
	final3.c
	finalplus.c
	formel64.c
	freezeframe.c
	freezeframe2.c
	freezemachine.c
	funplay.c
	gamekiller.c
	gmod2.c
	gmod3.c
	gs.c
	hyperbasic.c
	ide64.c
	ieeeflash64.c
	isepic.c
	kcs.c
	kingsoft.c
	ltkernal.c
	mach5.c
	magicdesk.c
	magicformel.c
	magicvoice.c
	magicdesk16.c
	maxbasic.c
	megabyter.c
	mikroass.c
	mmcreplay.c
	mmc64.c
	multimax.c
	ocean.c
	prophet64.c
	pagefox.c
	partner64.c
	profidos.c
	ramcart.c
	ramlink.c
	retroreplay.c
	reu.c
	rexep256.c
	rexramfloppy.c
	rexutility.c
	rgcd.c
	ross.c
	rrnetmk3.c
	sdbox.c
	shortbus.c
	shortbus_digimax.c
	silverrock128.c
	simonsbasic.c
	snapshot64.c
	stardos.c
	stb.c
	superexplode5.c
	supergames.c
	supersnapshot.c
	supersnapshot4.c
	turtlegraphics.c
	uc1.c
	uc2.c
	warpspeed.c
	westermann.c
	zaxxon.c
	zippcode48.c
)

addObjectLib(libc64commoncart "vice/c64/cart"
	c64acia1.c
	digimax.c
	ds12c887rtc.c
	georam.c
	sfx_soundexpander.c
	sfx_soundsampler.c
)

addObjects(libc64BaseObjs "vice/c64"
	c64-cmdline-options.c
	c64-memory-hacks.c
	c64-resources.c
	c64-snapshot.c
	c64.c
	c64_256k.c
	c64bus.c
	c64cia1.c
	c64cia2.c
	c64datasette.c
	c64drive.c
	c64export.c
	c64fastiec.c
	c64gluelogic.c
	c64iec.c
	c64io.c
	c64keyboard.c
	c64meminit.c
	c64memlimit.c
	c64memrom.c
	c64memsnapshot.c
	c64parallel.c
	c64pla.c
	c64printer.c
	c64rom.c
	c64romset.c
	c64rsuser.c
	c64sound.c
	c64video.c
	plus256k.c
	plus60k.c
	psid.c
	reloc65.c
)

addObjectLib(libc64Extra "vice/c64"
	c64cpu.c
	c64mem.c
	c64model.c
)
set(libc64Objs ${libc64BaseObjs} $<TARGET_OBJECTS:libc64Extra>)

addObjectLib(libc64scExtra "vice/c64"
	c64cpusc.c
	c64memsc.c
	c64scmodel.c
)
set(libc64scObjs ${libc64BaseObjs} $<TARGET_OBJECTS:libc64scExtra>)

set(libc64scpu64Objs
	$<TARGET_OBJECTS:c64bus.c>
	$<TARGET_OBJECTS:c64cia1.c>
	$<TARGET_OBJECTS:c64cia2.c>
	$<TARGET_OBJECTS:c64drive.c>
	$<TARGET_OBJECTS:c64export.c>
	$<TARGET_OBJECTS:c64fastiec.c>
	$<TARGET_OBJECTS:c64iec.c>
	$<TARGET_OBJECTS:c64io.c>
	$<TARGET_OBJECTS:c64keyboard.c>
	$<TARGET_OBJECTS:c64memsnapshot.c>
	$<TARGET_OBJECTS:c64parallel.c>
	$<TARGET_OBJECTS:c64printer.c>
	$<TARGET_OBJECTS:c64romset.c>
	$<TARGET_OBJECTS:c64rsuser.c>
	$<TARGET_OBJECTS:c64sound.c>
	$<TARGET_OBJECTS:c64video.c>
	$<TARGET_OBJECTS:reloc65.c>
)

set(libc64c64dtvObjs
	$<TARGET_OBJECTS:c64bus.c>
	$<TARGET_OBJECTS:c64drive.c>
	$<TARGET_OBJECTS:c64fastiec.c>
	$<TARGET_OBJECTS:c64keyboard.c>
	$<TARGET_OBJECTS:c64rom.c>
	$<TARGET_OBJECTS:c64romset.c>
	$<TARGET_OBJECTS:c64rsuser.c>
	$<TARGET_OBJECTS:c64video.c>
)

set(libc64c128Objs
	$<TARGET_OBJECTS:c64bus.c>
	$<TARGET_OBJECTS:c64cia2.c>
	$<TARGET_OBJECTS:c64datasette.c>
	$<TARGET_OBJECTS:c64export.c>
	$<TARGET_OBJECTS:c64gluelogic.c>
	$<TARGET_OBJECTS:c64iec.c>
	$<TARGET_OBJECTS:c64io.c>
	$<TARGET_OBJECTS:c64keyboard.c>
	$<TARGET_OBJECTS:c64meminit.c>
	$<TARGET_OBJECTS:c64memlimit.c>
	$<TARGET_OBJECTS:c64memrom.c>
	$<TARGET_OBJECTS:c64printer.c>
	$<TARGET_OBJECTS:c64pla.c>
	$<TARGET_OBJECTS:c64parallel.c>
	$<TARGET_OBJECTS:c64rsuser.c>
	$<TARGET_OBJECTS:c64sound.c>
)

addObjectLib(libviciiCommon "vice/vicii"
	vicii-badline.c
	vicii-clock-stretch.c
	vicii-cmdline-options.c
	vicii-fetch.c
	vicii-irq.c
	vicii-mem.c
	vicii-phi1.c
	vicii-resources.c
	vicii-sprites.c
	vicii-timing.c
	vicii.c
)

addObjectLib(libvicii "vice/vicii"
	vicii-color.c
	vicii-draw.c
	vicii-snapshot.c
	vicii-stubs.c
)
set(libviciiObjs $<TARGET_OBJECTS:libviciiCommon> $<TARGET_OBJECTS:libvicii>)

addObjectLib(libviciitv "vice/vicii"
	viciidtv-color.c
	viciidtv-draw.c
	viciidtv-snapshot.c
)
set(libviciitvObjs $<TARGET_OBJECTS:libviciiCommon> $<TARGET_OBJECTS:libviciitv>)

addObjectLib(libcbmBase "vice/cbm2"
	cbm2-cmdline-options.c
	cbm2acia1.c
	cbm2bus.c
	cbm2cpu.c
	cbm2datasette.c
	cbm2drive.c
	cbm2export.c
	cbm2iec.c
	cbm2io.c
	cbm2memsnapshot.c
	cbm2model.c
	cbm2romset.c
	cbm2sound.c
	cbm2tpi1.c
	cbm2tpi2.c
	cart/cbm2-generic.c
	cart/cbm2cart.c
	cart/debugcart.c
)

addObjectLib(libcbm2Extra "vice/cbm2"
	cbm2-resources.c
	cbm2-snapshot.c
	cbm2-stubs.c
	cbm2.c
	cbm2cia1.c
	cbm2mem.c
	cbm2printer.c
	cbm2rom.c
	cbm2video.c
)
set(libcbm2Objs $<TARGET_OBJECTS:libcbmBase> $<TARGET_OBJECTS:libcbm2Extra>)

addObjectLib(libcbm5x0Extra "vice/cbm2"
	cbm5x0-resources.c
	cbm5x0-snapshot.c
	cbm5x0-stubs.c
	cbm5x0.c
	cbm5x0cia1.c
	cbm5x0mem.c
	cbm5x0printer.c
	cbm5x0rom.c
	cbm5x0video.c
)
set(libcbm5x0Objs $<TARGET_OBJECTS:libcbmBase> $<TARGET_OBJECTS:libcbm5x0Extra>)

addObjectLib(libsid "vice/sid"
	fastsid.c
	sid-cmdline-options.c
	sid-resources.c
	sid-snapshot.c
	sid.c
)

addObjectLib(librtcBase "vice/core/rtc"
	bq4830y.c
	ds1202_1302.c
	ds1216e.c
	ds12c887.c
	ds1602.c
	pcf8583.c
	rtc-72421.c
	rtc.c
)

addObjectLib(librtcExtra "vice/core/rtc"
	ds1307.c
	rtc-58321a.c
)
set(librtcObjs $<TARGET_OBJECTS:librtcBase> $<TARGET_OBJECTS:librtcExtra>)

addObjectLib(libvideo "vice/video"
	render1x1.c
	render1x2.c
	render1x1pal.c
	render1x1ntsc.c
	render1x1rgbi.c
	render1x2rgbi.c
	video-canvas.c
	video-cmdline-options.c
	video-color.c
	video-render.c
	video-render-crtmono.c
	video-render-palntsc.c
	video-render-rgbi.c
	video-resources.c
	video-sound.c
	video-viewport.c
)

addObjectLib(librs232drvBase "vice/rs232drv/"
	rs232.c
	rs232drv.c
	rs232net.c
)

addObjectLib(librs232drvExtra "vice/rs232drv/"
	rsuser.c
)
set(librs232drvObjs $<TARGET_OBJECTS:librs232drvBase> $<TARGET_OBJECTS:librs232drvExtra>)

if(SUBENV STREQUAL pandora)
	set(libcCompatSrc linuxCompat.c)
elseif(ENV STREQUAL android AND ANDROID_COMPAT_API)
	set(libcCompatSrc androidCompat.c)
endif()

addObjectLib(libplugin "main" pluginCommon.c pluginCxxSupport.cc ${libcCompatSrc})
addObjectLib(libiecbusStubs "main" iecbusStubs.c)

file(GLOB libresidSrcs RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "vice/resid/*.cc")
list(FILTER libresidSrcs EXCLUDE REGEX "vice/resid/filter.cc")
list(APPEND libresidSrcs "vice/sid/resid.cc")
addObjectLib(libresid "" ${libresidSrcs})
target_compile_options(libresid PRIVATE -fno-exceptions -fno-rtti)
# needed to prevent miscompile when building with -ffast-math
set_source_files_properties(vice/resid/dac.cc PROPERTIES COMPILE_OPTIONS "-fno-finite-math-only")

addGlobObjectLib(libresiddtv "vice/resid-dtv/*.cc" "vice/sid/resid-dtv.cc")
target_compile_options(libresiddtv PRIVATE -fno-exceptions -fno-rtti)

addGlobObjectLib(libcrtc "vice/crtc/*.c")
addGlobObjectLib(libiecbus "vice/iecbus/*.c")
addGlobObjectLib(libserial "vice/serial/*.c")
addGlobObjectLib(libdatasette "vice/datasette/*.c")
addGlobObjectLib(libdrive "vice/drive/*.c")
addGlobObjectLib(libdriveiec "vice/drive/iec/*.c")
addGlobObjectLib(libdriveiecc64exp "vice/drive/iec/c64exp/*.c")
addGlobObjectLib(libdriveiecplus4exp "vice/drive/iec/plus4exp/*.c")
addGlobObjectLib(libdriveiec128dcr "vice/drive/iec128dcr/*.c")
addGlobObjectLib(libdriveiecieee "vice/drive/iecieee/*.c")
addGlobObjectLib(libdriveieee "vice/drive/ieee/*.c")
addGlobObjectLib(libdrivetcbm "vice/drive/tcbm/*.c")
addGlobObjectLib(libp64 "vice/lib/p64/*.c")
addGlobObjectLib(libimagecontents "vice/imagecontents/*.c")
addGlobObjectLib(libmonitor "vice/monitor/*.c")
addGlobObjectLib(libparallel "vice/parallel/*.c")
addGlobObjectLib(libvdrive "vice/vdrive/*.c")
addGlobObjectLib(libviciisc "vice/viciisc/*.c")
addGlobObjectLib(libvdc "vice/vdc/*.c")
addGlobObjectLib(libvic20 "vice/vic20/*.c" "vice/vic20/cart/*.c")
addGlobObjectLib(libpet "vice/pet/*.c")
addGlobObjectLib(libplus4 "vice/plus4/*.c" "vice/plus4/cart/*.c")
addGlobObjectLib(libtape "vice/tape/*.c")
addGlobObjectLib(libcore "vice/core/*.c")
addGlobObjectLib(libuserport "vice/userport/*.c")
addGlobObjectLib(libraster "vice/raster/*.c")
addGlobObjectLib(libfsdevice "vice/fsdevice/*.c")
addGlobObjectLib(libprinterdrv "vice/printerdrv/*.c")
addGlobObjectLib(libdiag "vice/diag/*.c")
addGlobObjectLib(libdiskimage "vice/diskimage/*.c")
addGlobObjectLib(libfileio "vice/fileio/*.c")
addGlobObjectLib(libjoyport "vice/joyport/*.c")
addGlobObjectLib(libtapeport "vice/tapeport/*.c")
addGlobObjectLib(libsamplerdrv "vice/samplerdrv/*.c")
addGlobObjectLib(libc64dtv "vice/c64dtv/*.c")
addGlobObjectLib(libc128 "vice/c128/*.c" "vice/c128/cart/*.c")
addGlobObjectLib(libscpu64 "vice/scpu64/*.c")

set(pluginNoTapeObjs
	$<TARGET_OBJECTS:libplugin>
	$<TARGET_OBJECTS:libprinterdrv>
	$<TARGET_OBJECTS:libdiskimage>
	$<TARGET_OBJECTS:libfsdevice>
	$<TARGET_OBJECTS:libfileio>
	$<TARGET_OBJECTS:libserial>
	$<TARGET_OBJECTS:libcore>
	$<TARGET_OBJECTS:libjoyport>
	$<TARGET_OBJECTS:libsamplerdrv>
	$<TARGET_OBJECTS:libdrivetcbm>
	$<TARGET_OBJECTS:libdiag>
	$<TARGET_OBJECTS:libbase>
	$<TARGET_OBJECTS:libvdrive>
	$<TARGET_OBJECTS:libdrive>
	$<TARGET_OBJECTS:libdriveiec>
	$<TARGET_OBJECTS:libdriveieee>
	$<TARGET_OBJECTS:libdriveiecieee>
	$<TARGET_OBJECTS:libp64>
	$<TARGET_OBJECTS:libimagecontents>
	$<TARGET_OBJECTS:libraster>
	$<TARGET_OBJECTS:libmonitor>
	$<TARGET_OBJECTS:libvideo>
	$<TARGET_OBJECTS:libsid>
)

set(pluginObjs
	${pluginNoTapeObjs}
	$<TARGET_OBJECTS:libtape>
	$<TARGET_OBJECTS:libdatasette>
)

set(pluginLinkLibs -lz -lm)

if(ENV STREQUAL android)
 # must link to the app's main shared object so Android resolves runtime symbols correctly
 list(APPEND pluginLinkLibs -llog c64emu)
endif()

function(addViceSharedLib target)
	configureAppLibraryTarget(${target} ${ARGN})
	target_link_libraries(${target} PRIVATE c64emuInterface ${pluginLinkLibs})
endfunction()

addViceSharedLib(c64
	${libc64Objs}
	$<TARGET_OBJECTS:libc64cartsystem>
	$<TARGET_OBJECTS:libc64cart>
	$<TARGET_OBJECTS:libc64commoncart>
	$<TARGET_OBJECTS:libdriveiecc64exp>
	$<TARGET_OBJECTS:libiecbus>
	$<TARGET_OBJECTS:libparallel>
	$<TARGET_OBJECTS:libuserport>
	$<TARGET_OBJECTS:libresid>
	$<TARGET_OBJECTS:libtapeport>
	${libviciiObjs}
	${librs232drvObjs}
	${librtcObjs}
	${pluginObjs}
)

addViceSharedLib(c64sc
	${libc64scObjs}
	$<TARGET_OBJECTS:libc64cartsystem>
	$<TARGET_OBJECTS:libc64cart>
	$<TARGET_OBJECTS:libc64commoncart>
	$<TARGET_OBJECTS:libdriveiecc64exp>
	$<TARGET_OBJECTS:libiecbus>
	$<TARGET_OBJECTS:libparallel>
	$<TARGET_OBJECTS:libviciisc>
	$<TARGET_OBJECTS:libuserport>
	$<TARGET_OBJECTS:libtapeport>
	$<TARGET_OBJECTS:libresid>
	${librs232drvObjs}
	${librtcObjs}
	${pluginObjs}
)

addViceSharedLib(scpu64
	$<TARGET_OBJECTS:libscpu64>
	${libc64scpu64Objs}
	$<TARGET_OBJECTS:libc64cartsystem>
	$<TARGET_OBJECTS:libc64cart>
	$<TARGET_OBJECTS:libc64commoncart>
	$<TARGET_OBJECTS:libdriveiecc64exp>
	$<TARGET_OBJECTS:libiecbus>
	$<TARGET_OBJECTS:libparallel>
	$<TARGET_OBJECTS:libviciisc>
	$<TARGET_OBJECTS:libuserport>
	$<TARGET_OBJECTS:libresid>
	${librs232drvObjs}
	${librtcObjs}
	${pluginNoTapeObjs}
	main/scpu64Stubs.c
)

addViceSharedLib(c64dtv
	$<TARGET_OBJECTS:libc64dtv>
	${libc64c64dtvObjs}
	$<TARGET_OBJECTS:libdriveiecc64exp>
	$<TARGET_OBJECTS:libiecbus>
	$<TARGET_OBJECTS:libparallel>
	${libviciitvObjs}
	$<TARGET_OBJECTS:libuserport>
	$<TARGET_OBJECTS:libresiddtv>
	$<TARGET_OBJECTS:librtcBase>
	$<TARGET_OBJECTS:librs232drvBase>
	${pluginObjs}
)

addViceSharedLib(c128
	$<TARGET_OBJECTS:libc128>
	${libc64c128Objs}
	$<TARGET_OBJECTS:libc64cartsystem>
	$<TARGET_OBJECTS:libc64cart>
	$<TARGET_OBJECTS:libc64commoncart>
	$<TARGET_OBJECTS:libdriveiec128dcr>
	$<TARGET_OBJECTS:libdriveiecc64exp>
	$<TARGET_OBJECTS:libiecbus>
	$<TARGET_OBJECTS:libparallel>
	$<TARGET_OBJECTS:libvdc>
	$<TARGET_OBJECTS:libuserport>
	$<TARGET_OBJECTS:libtapeport>
	$<TARGET_OBJECTS:libresid>
	${libviciiObjs}
	${librs232drvObjs}
	${librtcObjs}
	${pluginObjs}
)

addViceSharedLib(vic
	$<TARGET_OBJECTS:libvic20>
	$<TARGET_OBJECTS:libc64commoncart>
	$<TARGET_OBJECTS:libiecbus>
	$<TARGET_OBJECTS:libparallel>
	$<TARGET_OBJECTS:libuserport>
	$<TARGET_OBJECTS:libtapeport>
	$<TARGET_OBJECTS:libresid>
	${librs232drvObjs}
	${librtcObjs}
	${pluginObjs}
)

addViceSharedLib(pet
	$<TARGET_OBJECTS:libpet>
	$<TARGET_OBJECTS:libparallel>
	$<TARGET_OBJECTS:libcrtc>
	$<TARGET_OBJECTS:libuserport>
	$<TARGET_OBJECTS:libtapeport>
	$<TARGET_OBJECTS:libresid>
	$<TARGET_OBJECTS:libiecbusStubs>
	$<TARGET_OBJECTS:librs232drvBase>
	${librtcObjs}
	${pluginObjs}
)

addViceSharedLib(plus4
	$<TARGET_OBJECTS:libplus4>
	$<TARGET_OBJECTS:libdriveiecplus4exp>
	$<TARGET_OBJECTS:libiecbus>
	$<TARGET_OBJECTS:libuserport>
	$<TARGET_OBJECTS:libtapeport>
	$<TARGET_OBJECTS:libresid>
	${librs232drvObjs}
	${librtcObjs}
	${pluginObjs}
)

addViceSharedLib(cbm2
	${libcbm2Objs}
	$<TARGET_OBJECTS:libparallel>
	$<TARGET_OBJECTS:libcrtc>
	$<TARGET_OBJECTS:libuserport>
	$<TARGET_OBJECTS:libtapeport>
	$<TARGET_OBJECTS:libresid>
	$<TARGET_OBJECTS:libiecbusStubs>
	$<TARGET_OBJECTS:librs232drvBase>
	${librtcObjs}
	${pluginObjs}
)

addViceSharedLib(cbm5x0
	${libcbm5x0Objs}
	$<TARGET_OBJECTS:libparallel>
	$<TARGET_OBJECTS:libtapeport>
	$<TARGET_OBJECTS:libresid>
	$<TARGET_OBJECTS:libiecbusStubs>
	${libviciiObjs}
	${librs232drvObjs}
	${librtcObjs}
	${pluginObjs}
	main/cbm5x0Stubs.c
)
