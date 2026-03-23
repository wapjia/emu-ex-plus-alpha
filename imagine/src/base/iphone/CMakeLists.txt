addPkgConfigLibs(imagine "-framework=UIKit -framework=QuartzCore -framework=Foundation -framework=CoreFoundation -framework=CoreGraphics")

if(iosMsgUI)
	addConfigEnable(imagine IPHONE_MSG_COMPOSE)
	addPkgConfigLibs(imagine "-framework=MessageUI")
endif()

target_sources(
	imagine PRIVATE
	iphone.mm
	IOSWindow.mm
	IOSScreen.mm
	EAGLView.mm
	input.mm
	IOSGLContext.mm
	../common/timer/CFTimer.cc
	../common/PosixPipe.cc
	../common/SimpleFrameTimer.cc
	../common/eventloop/CFEventLoop.cc
	../common/eventloop/FDCustomEvent.cc
	../../util/string/apple.mm
)
