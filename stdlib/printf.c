// stdlib printf/vprintf — write formatted output to stdout.
//
// Implementation delegates to corec/base/numconv.c's base_vsnprintf, which
// already supports the full format set we need: %d, %i, %u, %ld, %lu, %lld,
// %llu, %zu, %x, %X, %lx, %lX, %llx, %llX, %p, %c, %s, %f, %.Nf, %%.
// Keeping a single formatter (base_vsnprintf) means printf and snprintf
// behave identically.
//
// The output is buffered into a single stack-sized chunk per call. If the
// formatted output would exceed PRINTF_BUF_SIZE-1 bytes, it is truncated
// before being written, but the *return value* still reports what would
// have been written if the buffer were large enough — matching ISO C
// snprintf semantics, applied to printf.

#include <stddef.h>
#include <stdint.h>
#include <printf.h>
#include <stdarg.h>
#include <base/io.h>
#include <base/numconv.h>
#include <platform/platform.h>

#define PRINTF_BUF_SIZE 4096

int vprintf(const char* format, va_list ap) {
    char buf[PRINTF_BUF_SIZE];
    int n = base_vsnprintf(buf, sizeof(buf), format, ap);
    if (n <= 0) {
        return n;
    }

    size_t to_write = (size_t)n;
    if (to_write > sizeof(buf) - 1) {
        to_write = sizeof(buf) - 1;
    }

    ciovec_t iov;
    iov.buf = buf;
    iov.buf_len = to_write;
    write_all(PLATFORM_STDOUT_FD, &iov, 1);

    return n;
}

int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);
    va_end(args);
    return result;
}
