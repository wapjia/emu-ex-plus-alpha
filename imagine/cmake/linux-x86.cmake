include("${CMAKE_CURRENT_LIST_DIR}/config.cmake")

set(CTARGET x86_64-pc-linux-gnu)
set(ARCH x86)
set(CFLAGS_CODEGEN "-m32 -march=pentium4 -mtune=generic")
set(CMAKE_ASM_FLAGS_INIT "-m32 -O3")
set(SANITIZE_MODE "address,undefined")

include("${CMAKE_CURRENT_LIST_DIR}/linux.cmake")
