#pragma once

// Minimal <ctype.h> — character classification and case mapping.
//
// All functions accept an int whose value must be representable as
// `unsigned char` or be EOF (-1), per ISO C. They are implemented as
// inline functions using ASCII semantics, which matches the "C" locale
// — this layer does not implement non-C locales.

static inline int isdigit(int c) { return c >= '0' && c <= '9'; }
static inline int isxdigit(int c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}
static inline int isupper(int c) { return c >= 'A' && c <= 'Z'; }
static inline int islower(int c) { return c >= 'a' && c <= 'z'; }
static inline int isalpha(int c) { return isupper(c) || islower(c); }
static inline int isalnum(int c) { return isalpha(c) || isdigit(c); }
static inline int isspace(int c) {
    return c == ' '  || c == '\t' || c == '\n' ||
           c == '\v' || c == '\f' || c == '\r';
}
static inline int iscntrl(int c) { return (c >= 0 && c < 32) || c == 127; }
static inline int isprint(int c) { return c >= 32 && c < 127; }
static inline int isgraph(int c) { return c > 32 && c < 127; }
static inline int ispunct(int c) {
    return isprint(c) && !isalnum(c) && c != ' ';
}

static inline int toupper(int c) {
    return islower(c) ? c - ('a' - 'A') : c;
}
static inline int tolower(int c) {
    return isupper(c) ? c + ('a' - 'A') : c;
}
