include("${CMAKE_CURRENT_LIST_DIR}/android-config.cmake")

set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
set(ARCH aarch64)
set(SUBARCH arm64)
set(CTARGET aarch64-linux-android)
set(ANDROID_CTARGET aarch64-none-linux-android21)
set(ANDROID_NDK_SDK 21)
set(LDFLAGS "-Wl,-z,max-page-size=16384")

include("${CMAKE_CURRENT_LIST_DIR}/android.cmake")
