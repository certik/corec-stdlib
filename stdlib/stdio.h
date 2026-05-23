#pragma once

#include <stdarg.h>
#include <stddef.h>

// Minimal FILE I/O.
// FILE is an opaque type; this layer keeps a small static pool of FILE
// objects (see stdlib/stdio.c). Only a handful of operations are
// implemented — enough to cover the common "open / read / seek / close"
// pattern used by simple data loaders.
#if !defined(FILE_DECLARED)
#define FILE_DECLARED
typedef struct FILE FILE;
#endif

// printf family — write formatted output to stdout.
int printf(const char *format, ...);
int vprintf(const char *format, va_list ap);

// fprintf family — write formatted output to a specific FILE *.
int fprintf(FILE *stream, const char *format, ...);
int vfprintf(FILE *stream, const char *format, va_list ap);

// sprintf family — write formatted output into an unbounded char buffer.
// Caller must ensure the buffer is large enough; use snprintf when in
// doubt.
int sprintf(char *str, const char *format, ...);
int vsprintf(char *str, const char *format, va_list ap);

// snprintf family — write formatted output to a fixed-size buffer.
// Returns the number of characters that would have been written if the
// buffer had been large enough (matches ISO C snprintf).
int snprintf(char *str, size_t size, const char *format, ...);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

// Returned by character / line-oriented I/O functions to signal end of file
// or an error. Standard C requires this to be a negative integer constant
// expression that fits in an int.
#define EOF (-1)

FILE *fopen(const char *filename, const char *mode);
int   fclose(FILE *stream);
int   fseek(FILE *stream, long offset, int whence);
long  ftell(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int   fputc(int c, FILE *stream);
int   fputs(const char *s, FILE *stream);

// Standard streams — lazily initialised on first use. Backed by
// PLATFORM_{STDIN,STDOUT,STDERR}_FD via the platform layer. Treat as
// non-NULL FILE pointers; do not fclose() them.
FILE *_std_stream(int which);
#define stdin  (_std_stream(0))
#define stdout (_std_stream(1))
#define stderr (_std_stream(2))
