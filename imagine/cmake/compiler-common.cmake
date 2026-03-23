if(NOT SUBARCH)
	set(SUBARCH ${ARCH})
endif()

if(NOT IMAGINE_SDK_PLATFORM)
	if(SUBENV)
		set(IMAGINE_SDK_PLATFORM ${ENV}-${SUBARCH}-${SUBENV})
	else()
		set(IMAGINE_SDK_PLATFORM ${ENV}-${SUBARCH})
	endif()
endif()

set(IMAGINE_SDK_PLATFORM_PATH "${IMAGINE_SDK_PATH}/${IMAGINE_SDK_PLATFORM}")
set(PKG_CONFIG_PATH "${IMAGINE_SDK_PLATFORM_PATH}/lib/pkgconfig:${PKG_CONFIG_PATH}")
if(NOT CMAKE_SCRIPT_MODE_FILE)
	include_directories("${IMAGINE_SDK_PLATFORM_PATH}/include")
	link_directories("${IMAGINE_SDK_PLATFORM_PATH}/lib")
endif()

if(USE_EXTERNAL_LIBCXX)
	set(CXX_STD_LINK_OPTS -nostdlib++)
	set(CXX_STD_LINK_LIBS -lc++ -lc++abi -lc++experimental ${LIBCXX_SUPPORT_LIBS})
	string(APPEND CXXFLAGS " -nostdinc++ -isystem ${IMAGINE_SDK_PLATFORM_PATH}/include/c++/v1")
	set(CMAKE_CXX_COMPILER_ID_ARG1 -B "${IMAGINE_SDK_PLATFORM_PATH}/lib") # used to locate libc++.modules.json in Clang-CXX-CXXImportStd.cmake
endif()

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CFLAGS_DEBUG "-Og")
set(CFLAGS_OPTIMIZE "-ffast-math -flto -fno-stack-protector -fno-asynchronous-unwind-tables")
string(APPEND CFLAGS_CODEGEN " -fvisibility=hidden -fno-semantic-interposition -gz -ffunction-sections -fdata-sections")
string(APPEND CFLAGS_COMMON " -pipe -Wall -Wextra")
string(APPEND CFLAGS_COMMON_RELEASE " -Wdisabled-optimization")
string(APPEND CXXFLAGS " -fno-threadsafe-statics")
if(NOT LDFLAGS_STRIP)
	set(LDFLAGS_STRIP "-s")
endif()

if(NOT USE_FRAME_POINTERS)
	string(APPEND CFLAGS_OPTIMIZE " -fomit-frame-pointer")
endif()

if(NOT NO_SANITIZE_MODE)
	# Disable some -fsanitize=undefined options that greatly increase compile time or are not needed
	set(NO_SANITIZE_MODE "unreachable,return,vptr,enum,nonnull-attribute")
endif()

if(SANITIZE_MODE)
	string(APPEND CFLAGS_DEBUG " -fsanitize=${SANITIZE_MODE} -fno-omit-frame-pointer")
	if(NO_SANITIZE_MODE)
		string(APPEND CFLAGS_DEBUG " -fno-sanitize=${NO_SANITIZE_MODE}")
	endif()
endif()

# assign all the flags

set(CMAKE_C_FLAGS_INIT "${CFLAGS_COMMON} ${CFLAGS} ${CFLAGS_CODEGEN}")
set(CMAKE_C_FLAGS_DEBUG_INIT "${CFLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE_INIT "${CFLAGS_COMMON_RELEASE} ${CFLAGS_OPTIMIZE}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "${CMAKE_C_FLAGS_RELEASE_INIT}")

set(CMAKE_CXX_FLAGS_INIT "${CFLAGS_COMMON} ${CXXFLAGS} ${CFLAGS_CODEGEN}")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "${CFLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CFLAGS_COMMON_RELEASE} ${CFLAGS_OPTIMIZE}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "${CMAKE_CXX_FLAGS_RELEASE_INIT}")

set(CMAKE_EXE_LINKER_FLAGS_INIT "${LDFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT "-flto") # Ensure LTO files are processed properly even when not compiling with LTO
set(CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT "${LDFLAGS_STRIP}")

set(CMAKE_SHARED_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} ${LDFLAGS_SHARED}")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT "${CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT}")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE_INIT "${CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT}")