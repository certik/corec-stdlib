#pragma once

// <limits.h> — sizes of integer types.
//
// Values are platform-dependent for `long` and `long long`. We follow the
// standard data models used by our supported platforms:
//
//   * Windows x64           — LLP64: long is 32-bit, long long 64-bit.
//   * Linux/macOS x86_64/ARM64 — LP64:  long is 64-bit, long long 64-bit.
//   * WebAssembly (wasm32)  — ILP32: long is 32-bit, long long 64-bit.

#define CHAR_BIT  8

#define SCHAR_MIN (-128)
#define SCHAR_MAX 127
#define UCHAR_MAX 255

// `char`'s signedness is implementation-defined. On every platform we
// support today (Linux, macOS, Windows, WASM) plain char is signed.
#define CHAR_MIN  SCHAR_MIN
#define CHAR_MAX  SCHAR_MAX

#define SHRT_MIN  (-32768)
#define SHRT_MAX  32767
#define USHRT_MAX 65535

#define INT_MIN   (-INT_MAX - 1)
#define INT_MAX   2147483647
#define UINT_MAX  4294967295u

#if defined(__LP64__) || defined(_LP64)
#define LONG_MAX  9223372036854775807L
#define LONG_MIN  (-LONG_MAX - 1L)
#define ULONG_MAX 18446744073709551615UL
#else
#define LONG_MAX  2147483647L
#define LONG_MIN  (-LONG_MAX - 1L)
#define ULONG_MAX 4294967295UL
#endif

#define LLONG_MAX  9223372036854775807LL
#define LLONG_MIN  (-LLONG_MAX - 1LL)
#define ULLONG_MAX 18446744073709551615ULL
