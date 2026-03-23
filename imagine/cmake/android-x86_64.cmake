include("${CMAKE_CURRENT_LIST_DIR}/android-config.cmake")

set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_ANDROID_ARCH_ABI x86_64)
set(ARCH x86_64)
set(CTARGET x86_64-linux-android)
set(ANDROID_CTARGET x86_64-none-linux-android21)
set(ANDROID_NDK_SDK 21)

include("${CMAKE_CURRENT_LIST_DIR}/android.cmake")
