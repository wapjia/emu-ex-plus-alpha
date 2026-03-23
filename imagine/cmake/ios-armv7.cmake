include("${CMAKE_CURRENT_LIST_DIR}/config.cmake")

set(CMAKE_SYSTEM_PROCESSOR armv7-a)
set(ARCH arm)
set(SUBARCH armv7)
set(MIN_IOS_VERSION 6.0)
set(CMAKE_ASM_FLAGS_INIT "-arch armv7")
set(CFLAGS_CODEGEN "-arch armv7 -mthumb -mdynamic-no-pic -faligned-allocation")
set(LDFLAGS "-Wl,-no_pie")
set(USE_FRAME_POINTERS 1)

include("${CMAKE_CURRENT_LIST_DIR}/ios.cmake")
