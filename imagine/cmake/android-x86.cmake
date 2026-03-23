include("${CMAKE_CURRENT_LIST_DIR}/android-config.cmake")

set(CMAKE_SYSTEM_PROCESSOR i686)
set(CMAKE_ANDROID_ARCH_ABI x86)
set(ARCH x86)
set(CTARGET i686-linux-android)
# Must declare min API 21 to compile with NDK r26+ headers
set(ANDROID_CTARGET i686-none-linux-android21)
set(ANDROID_NDK_SDK 16)
set(CFLAGS_CODEGEN "-mstackrealign")
set(LIBCXX_SUPPORT_LIBS "-landroid_support")

include("${CMAKE_CURRENT_LIST_DIR}/android.cmake")
