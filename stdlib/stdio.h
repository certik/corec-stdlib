#pragma once

#include <stdarg.h>
#include <stddef.h>

// printf family — write formatted output to stdout.
int printf(const char *format, ...);
int vprintf(const char *format, va_list ap);

// snprintf family — write formatted output to a fixed-size buffer.
// Returns the number of characters that would have been written if the
// buffer had been large enough (matches ISO C snprintf).
int snprintf(char *str, size_t size, const char *format, ...);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);

// Minimal FILE I/O.
// FILE is an opaque type; this layer keeps a small static pool of FILE
// objects (see stdlib/stdio.c). Only a handful of operations are
// implemented — enough to cover the common "open / read / seek / close"
// pattern used by simple data loaders.
#if !defined(FILE_DECLARED)
#define FILE_DECLARED
typedef struct FILE FILE;
#endif

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

FILE *fopen(const char *filename, const char *mode);
int   fclose(FILE *stream);
int   fseek(FILE *stream, long offset, int whence);
long  ftell(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
