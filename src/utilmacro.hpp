#pragma once

#ifdef _MSC_VER
#define ALWAYS_INLINE __forceinline
#elif defined __GNUC__
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#endif