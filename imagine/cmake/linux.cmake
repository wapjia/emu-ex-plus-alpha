set(CMAKE_SYSTEM_NAME Linux)
set(ENV linux)
set(ENV_KERNEL linux)
set(WINDOW_SYSTEM x11)
if(NOT PACK_DYN_RELOCS)
	set(PACK_DYN_RELOCS relr)
endif()

string(APPEND CFLAGS_COMMON " -D_GNU_SOURCE")
string(APPEND CFLAGS_CODEGEN " -pthread")
string(APPEND LDFLAGS " -lm -fuse-ld=mold \
-Wl,-O3,--gc-sections,--as-needed,--icf=all,--exclude-libs=ALL,--pack-dyn-relocs=${PACK_DYN_RELOCS}")

if(NOT CMAKE_C_COMPILER)
	set(CMAKE_C_COMPILER ${CTARGET}-gcc-16)
	set(CMAKE_CXX_COMPILER ${CTARGET}-g++-16)
	set(CMAKE_AR ${CTARGET}-gcc-ar)
	set(CMAKE_RANLIB ${CTARGET}-gcc-ranlib)
	set(CMAKE_STRIP ${CTARGET}-strip)
	set(CMAKE_OBJDUMP ${CTARGET}-objdump)
endif()

string(FIND "${CMAKE_C_COMPILER}" clang clangSubStringPos)
if(${clangSubStringPos} GREATER -1)
	# Glibc's fortify source macro causes internal linkage errors when building the std module
	string(APPEND CXXFLAGS " -stdlib=libc++ -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0")
	include("${CMAKE_CURRENT_LIST_DIR}/clang.cmake")
else()
	# Glibc's fortify source macro causes an ICE with memset() & memcpy() from the std module, GCC bug #124477
	string(APPEND CXXFLAGS " -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0")
	include("${CMAKE_CURRENT_LIST_DIR}/gcc.cmake")
endif()
