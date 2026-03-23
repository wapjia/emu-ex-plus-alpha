if(ENV STREQUAL linux)
	add_subdirectory(linux)
	if(WINDOW_SYSTEM STREQUAL x11)
		add_subdirectory(x11)
	endif()
elseif(ENV STREQUAL android)
	add_subdirectory(android)
elseif(ENV STREQUAL ios)
	add_subdirectory(iphone)
elseif(ENV STREQUAL macosx)
	add_subdirectory(osx)
endif()

if(UNIX)
	target_sources(imagine PRIVATE common/library/posix.cc)
endif()

target_sources(
	imagine PRIVATE
	common/ApplicationContext.cc
	common/Application.cc
	common/Screen.cc
	common/Window.cc
	../pixmap/Pixmap.cc
	../thread/thread.cc
)
