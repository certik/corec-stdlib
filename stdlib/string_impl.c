// stdlib/string_impl.c — wrappers exposing the standard <string.h> names
// (memcpy, strlen, ...) on top of corec's base_* implementations.
//
// File name: this file is named string_impl.c (not string.c) because on
// Windows MSVC the object file basename collides with corec/base/string.c's
// string.obj when both end up in the same link.
//
// Note on memcpy/memset: the compiler may emit *implicit* calls to memcpy
// and memset even with -fno-builtin (e.g. for struct copies, array
// initializers, or memcpy intrinsics in libcalls). The corec platform
// layer therefore also provides memcpy/memset where required (with weak
// linkage on Linux, so this file's strong definitions take precedence).
// We define them here unconditionally — every nostdlib build that links
// stdlib/string_impl.c will get one, and only one, real symbol.

#include <string.h>
#include <base/mem.h>

size_t strlen(const char *str) {
    return base_strlen(str);
}

char *strcpy(char *dest, const char *src) {
    return base_strcpy(dest, src);
}

int strcmp(const char *s1, const char *s2) {
    return base_strcmp(s1, s2);
}

void *memcpy(void *dest, const void *src, size_t n) {
    return base_memcpy(dest, src, n);
}

void *memmove(void *dest, const void *src, size_t n) {
    return base_memmove(dest, src, n);
}

int memcmp(const void *s1, const void *s2, size_t n) {
    return base_memcmp(s1, s2, n);
}

void *memset(void *s, int c, size_t n) {
    return base_memset(s, c, n);
}

void *memchr(const void *s, int c, size_t n) {
    return base_memchr(s, c, n);
}

char *strchr(const char *s, int c) {
    return base_strchr(s, c);
}

char *strrchr(const char *s, int c) {
    return base_strrchr(s, c);
}

char *strncpy(char *dest, const char *src, size_t n) {
    return base_strncpy(dest, src, n);
}

size_t strcspn(const char *s, const char *reject) {
    return base_strcspn(s, reject);
}

int strncmp(const char *s1, const char *s2, size_t n) {
    return base_strncmp(s1, s2, n);
}

char *strstr(const char *haystack, const char *needle) {
    return base_strstr(haystack, needle);
}
