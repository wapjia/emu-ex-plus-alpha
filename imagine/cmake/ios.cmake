set(CMAKE_SYSTEM_NAME iOS)
set(APPLE_PLATFORM 1)
set(ENV ios)
set(ENV_KERNEL mach)
string(APPEND OBJCFLAGS " -fobjc-arc")
string(APPEND CFLAGS_COMMON " -Wno-error=deprecated-declarations")

set(CMAKE_ASM_COMPILER "as")

set(CCTOOLS_TOOCHAIN_PATH $ENV{CCTOOLS_TOOCHAIN_PATH})

if(CCTOOLS_TOOCHAIN_PATH)
	set(CMAKE_AR llvm-ar)
	set(CMAKE_RANLIB llvm-ranlib)
	file(GLOB ccToolsClangPath "${CCTOOLS_TOOCHAIN_PATH}/bin/*-clang")
	list(GET ccToolsClangPath 0 ccToolsClangPath)
	if(NOT CMAKE_SCRIPT_MODE_FILE)
		message("cctools-port Compiler: ${ccToolsClangPath}")
	endif()
	set(CMAKE_C_COMPILER "${ccToolsClangPath}")
	set(CMAKE_CXX_COMPILER "${CMAKE_C_COMPILER}++")
	set(CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS "${IMAGINE_PATH}/tools/clang-scan-deps-apple.sh")
	set(iosSimulatorSDKsPath "${CCTOOLS_TOOCHAIN_PATH}/SDK")
	set(iosSDKsPath "${CCTOOLS_TOOCHAIN_PATH}/SDK")
	# Use --no-default-config to prevent distro's Clang config from adding its flags to the build
	string(APPEND CFLAGS_CODEGEN " --no-default-config")
else()
	if(NOT ENV{CXX})
		set(CMAKE_C_COMPILER clang)
		set(CMAKE_CXX_COMPILER clang++)
	endif()
	set(CMAKE_AR ar)
	set(XCODE_PATH $ENV{XCODE_PATH})
	if(NOT XCODE_PATH)
		execute_process(
			COMMAND xcode-select --print-path
			OUTPUT_VARIABLE XCODE_PATH
			OUTPUT_STRIP_TRAILING_WHITESPACE
			COMMAND_ERROR_IS_FATAL ANY
		)
	endif()
	set(iosSimulatorSDKsPath "${XCODE_PATH}/Platforms/iPhoneSimulator.platform/Developer/SDKs")
	set(iosSDKsPath "${XCODE_PATH}/Platforms/iPhoneOS.platform/Developer/SDKs")
	string(APPEND LDFLAGS " -fobjc-arc")
endif()

if(CMAKE_CXX_COMPILER)
	set(CMAKE_OBJCXX_COMPILER ${CMAKE_CXX_COMPILER})
endif()

execute_process(
	COMMAND ${CMAKE_C_COMPILER} -arch ${SUBARCH} -dumpmachine
	OUTPUT_VARIABLE CTARGET
	OUTPUT_STRIP_TRAILING_WHITESPACE
	COMMAND_ERROR_IS_FATAL ANY
)

set(IOS_SDK $ENV{IOS_SDK})

if(ARCH STREQUAL x86)
	if(NOT CMAKE_OSX_SYSROOT)
 		if(IOS_SDK)
			set(CMAKE_OSX_SYSROOT "${iosSimulatorSDKsPath}/iPhoneSimulator${IOS_SDK}.sdk")
		else()
			file(GLOB iosSimulatorSDKPath "${iosSimulatorSDKsPath}/iPhoneSimulator*.sdk")
			list(GET iosSimulatorSDKPath 0 CMAKE_OSX_SYSROOT)
		endif()
	endif()
	string(APPEND CFLAGS_CODEGEN " -isysroot ${CMAKE_OSX_SYSROOT} -mios-simulator-version-min=${MIN_IOS_VERSION}")
	string(APPEND OBJCFLAGS " -fobjc-abi-version=2 -fobjc-legacy-dispatch")
else()
	if(NOT CMAKE_OSX_SYSROOT)
  		if(IOS_SDK)
			set(CMAKE_OSX_SYSROOT "${iosSDKsPath}/iPhoneOS${IOS_SDK}.sdk")
		else()
			file(GLOB iosSimulatorSDKPath "${iosSDKsPath}/iPhoneOS*.sdk")
			list(GET iosSimulatorSDKPath 0 CMAKE_OSX_SYSROOT)
		endif()
	endif()
	string(APPEND CFLAGS_CODEGEN " -isysroot ${CMAKE_OSX_SYSROOT} -miphoneos-version-min=${MIN_IOS_VERSION}")
endif()

string(APPEND LDFLAGS " -dead_strip -Wl,-dead_strip_dylibs")
string(APPEND CFLAGS_COMMON_RELEASE " -DNS_BLOCK_ASSERTIONS")
set(LDFLAGS_STRIP "-Wl,-x,-S")
set(LDFLAGS_SHARED "-bundle -flat_namespace -undefined suppress")

# libc++
set(USE_EXTERNAL_LIBCXX 1)
if(USE_EXTERNAL_LIBCXX)
	string(APPEND CFLAGS_COMMON " -D_LIBCPP_DISABLE_AVAILABILITY")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/clang.cmake")

set(CMAKE_OBJCXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} ${OBJCFLAGS}")
