addPkgX11(imagine)
addPkgEgl(imagine)

if(SUBENV STREQUAL pandora)
	addPkgConfigDeps(imagine "xdmcp xau xcb-xkb xcb-xfixes")
else()
	addPkgXRandr(imagine)
endif()

target_sources(imagine PRIVATE FILE_SET CXX_MODULES FILES xutils.ccm)

target_sources(
	imagine PRIVATE
	Application.cc
	ApplicationContext.cc
	XWindow.cc
	XScreen.cc
	xdnd.cc
	input.cc
	FrameTimer.cc
	XGLContextEGL.cc
	../common/EGLContextBase.cc
)
