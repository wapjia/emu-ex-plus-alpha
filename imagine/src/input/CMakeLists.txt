if(ENV STREQUAL linux)
	add_subdirectory(evdev)
elseif(ENV STREQUAL ios)
	add_subdirectory(apple)
endif()

target_sources(imagine PRIVATE FILE_SET CXX_MODULES FILES PackedInputAccess.ccm)

target_sources(
	imagine PRIVATE
	Axis.cc
	Input.cc
	Event.cc
	Device.cc
	DragTracker.cc
)
