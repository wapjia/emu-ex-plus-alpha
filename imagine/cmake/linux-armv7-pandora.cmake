include("${CMAKE_CURRENT_LIST_DIR}/config.cmake")

set(CTARGET arm-none-linux-gnueabi)
set(ARCH arm)
set(SUBARCH armv7)
set(SUBENV pandora)
set(OPENGL_API gles)
set(pandoraSDKSysroot "$ENV{PNDSDK}")

if(NOT IS_DIRECTORY "${pandoraSDKSysroot}/usr/lib")
	message(FATAL_ERROR "${pandoraSDKSysroot}/usr/lib not found, Please set PNDSDK to your Pandora SDK root directory")
endif()

set(PKG_CONFIG_PATH "${pandoraSDKSysroot}/usr/lib/pkgconfig")
set(PKG_CONFIG_SYSTEM_INCLUDE_PATH "${pandoraSDKSysroot}/usr/include")
set(PKG_CONFIG_SYSTEM_LIBRARY_PATH "${pandoraSDKSysroot}/usr/lib")
set(CXXFLAGS -Wno-register)
set(CFLAGS_CODEGEN "-mcpu=cortex-a8 -mfpu=neon -fno-stack-protector")
set(LDFLAGS "--sysroot=${pandoraSDKSysroot} -s -Wl,-rpath-link=${pandoraSDKSysroot}/usr/lib -lpthread -lrt -ldl")
set(CXX_STD_LINK_OPTS -static-libstdc++)
set(PACK_DYN_RELOCS none)

# don't use FORTIFY_SOURCE to avoid linking in newer glibc symbols
set(CFLAGS_COMMON "--sysroot=${pandoraSDKSysroot} \
-isystem /usr/lib/gcc/${CTARGET}/16/include/g++-v16 \
-isystem /usr/${CTARGET}/usr/include \
-isystem ${pandoraSDKSysroot}/usr/include \
-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 \
-Wno-psabi -Wno-unused-value")

include("${CMAKE_CURRENT_LIST_DIR}/linux.cmake")
