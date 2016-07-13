#pragma once

#ifndef DEBUG
#ifdef _MSC_VER
#define ALWAYS_INLINE __forceinline
#elif defined __GNUC__
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#else
#define ALWAYS_INLINE inline
#endif
#else
#define ALWAYS_INLINE inline
#endif