include("${CMAKE_CURRENT_LIST_DIR}/config.cmake")

set(CTARGET x86_64-pc-linux-gnu)
set(ARCH x86_64)
set(CFLAGS_CODEGEN "-m64 -march=x86-64-v3 -mtune=generic")
set(CMAKE_ASM_FLAGS_INIT "-m64 -O3")
set(SANITIZE_MODE "address,undefined")

include("${CMAKE_CURRENT_LIST_DIR}/linux.cmake")
