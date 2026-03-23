include("${CMAKE_CURRENT_LIST_DIR}/config.cmake")

set(ANDROID_NDK_PATH "$ENV{ANDROID_NDK_PATH}")

if(NOT ANDROID_NDK_PATH)
	set(studioNDKBasePath "$ENV{ANDROID_SDK_ROOT}")
	if(NOT studioNDKBasePath)
		set(studioNDKBasePath "$ENV{ANDROID_HOME}")
	endif()
	#message("Checking for side-by-side NDK in path: ${studioNDKBasePath}")
	file(GLOB ndkVersions "${studioNDKBasePath}/ndk/*")
	list(SORT ndkVersions ORDER DESCENDING)
	# choose the newest NDK version
	list(GET ndkVersions 0 newestNDKVersion)
	set(ANDROID_NDK_PATH "${newestNDKVersion}")
endif()

if(NOT IS_DIRECTORY "${ANDROID_NDK_PATH}/toolchains/llvm")
	message(FATAL_ERROR "Can't find Android NDK in path: ${ANDROID_NDK_PATH}, please set ANDROID_SDK_ROOT or ANDROID_HOME to your SDK root path, or set ANDROID_NDK_PATH to your NDK root path")
endif()

set(CMAKE_ANDROID_NDK "${ANDROID_NDK_PATH}")

function(printConfigInfo)
	_printConfigInfo()
	message("Android NDK path: ${ANDROID_NDK_PATH}")
	message("NDK Clang path: ${ANDROID_CLANG_TOOLCHAIN_BIN_PATH}")
	if(android_ndkLinkSysroot)
		message("NDK link sysroot path: ${android_ndkLinkSysroot}")
	endif()
endFunction()
