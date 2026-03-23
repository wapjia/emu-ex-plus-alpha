addPkgGlib(imagine)

target_sources(
	imagine PRIVATE
	Application.cc
	ApplicationContext.cc
	FBDevFrameTimer.cc
	../common/SimpleFrameTimer.cc
	../common/timer/TimerFD.cc
	../common/PosixPipe.cc
	../common/eventloop/FDCustomEvent.cc
	../common/eventloop/GlibEventLoop.cc
)

if(SUBENV STREQUAL pandora)
	target_sources(imagine PRIVATE compat.c)
else() # Linux Desktop
	target_sources(imagine PRIVATE dbus.cc DRMFrameTimer.cc)
	addPkgLibdrm(imagine)
	addPkgGio(imagine)
endif()
