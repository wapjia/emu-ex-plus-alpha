#pragma once

#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#ifdef __clang__
#define CLANG_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#endif

#ifdef __cplusplus
#define CLINK extern "C"
#define BEGIN_C_DECLS extern "C" {
#define END_C_DECLS }
#else
#define CLINK
#define BEGIN_C_DECLS
#define END_C_DECLS
#endif

// Make symbol remain visible after linking
#define LVISIBLE __attribute__((visibility("default")))
