include("${CMAKE_CURRENT_LIST_DIR}/config.cmake")

set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(ARCH aarch64)
set(SUBARCH arm64)
set(MIN_IOS_VERSION 7.0)
set(CMAKE_ASM_FLAGS_INIT "-arch arm64")
# TODO: remove -faligned-allocation when min iOS target is above 11.0
set(CFLAGS_CODEGEN "-arch arm64 -faligned-allocation")

include("${CMAKE_CURRENT_LIST_DIR}/ios.cmake")
