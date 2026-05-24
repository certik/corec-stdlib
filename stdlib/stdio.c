#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>

#include <base/io.h>
#include <base/numconv.h>
#include <platform/platform.h>

// FILE structure wrapping a corec platform file descriptor
typedef struct FILE {
    platform_fd_t fd;
    int eof;
    int error;
} FILE;

// Statically allocate a small pool of FILE structures.
// Increase if needed; allocating dynamically would require malloc/free here,
// which we deliberately avoid in the stdio layer.
#define FILE_POOL_SIZE 16
static FILE file_pool[FILE_POOL_SIZE];
static int file_pool_initialized = 0;

// Standard streams. Initialised lazily on first use of stdin/stdout/stderr.
// Backed by platform-stable fd numbers (PLATFORM_STDIN_FD / STDOUT_FD /
// STDERR_FD). Each `FILE` is the same statically-allocated object returned
// to user code via the `stdin` / `stdout` / `stderr` macros.
static FILE _std_streams[3];
static int  _std_streams_initialized = 0;
static FILE *_init_std_streams(int which) {
    if (!_std_streams_initialized) {
        _std_streams[0].fd = PLATFORM_STDIN_FD;
        _std_streams[1].fd = PLATFORM_STDOUT_FD;
        _std_streams[2].fd = PLATFORM_STDERR_FD;
        for (int i = 0; i < 3; i++) { _std_streams[i].eof = 0; _std_streams[i].error = 0; }
        _std_streams_initialized = 1;
    }
    return &_std_streams[which];
}
FILE *_std_stream(int which) { return _init_std_streams(which); }

static void init_file_pool(void) {
    if (!file_pool_initialized) {
        for (int i = 0; i < FILE_POOL_SIZE; i++) {
            file_pool[i].fd = -1;
            file_pool[i].eof = 0;
            file_pool[i].error = 0;
        }
        file_pool_initialized = 1;
    }
}

static FILE* alloc_file(platform_fd_t fd) {
    init_file_pool();
    for (int i = 0; i < FILE_POOL_SIZE; i++) {
        if (file_pool[i].fd == -1) {
            file_pool[i].fd = fd;
            file_pool[i].eof = 0;
            file_pool[i].error = 0;
            return &file_pool[i];
        }
    }
    return NULL;
}

static void free_file(FILE* file) {
    if (file) {
        file->fd = -1;
        file->eof = 0;
        file->error = 0;
    }
}

FILE *fopen(const char *filename, const char *mode) {
    // Parse mode string
    uint64_t rights = PLATFORM_RIGHTS_READ;
    int oflags = 0;

    if (mode[0] == 'r') {
        rights = PLATFORM_RIGHTS_READ;
    } else if (mode[0] == 'w') {
        rights = PLATFORM_RIGHTS_WRITE;
        oflags = PLATFORM_O_CREAT | PLATFORM_O_TRUNC;
    } else if (mode[0] == 'a') {
        rights = PLATFORM_RIGHTS_WRITE;
        oflags = PLATFORM_O_CREAT;
    } else {
        return NULL;
    }

    // Check for binary mode (ignored, all files are binary)
    // Check for + mode (read/write)
    for (int i = 1; mode[i]; i++) {
        if (mode[i] == '+') {
            rights = PLATFORM_RIGHTS_RDWR;
        }
    }

    // Calculate filename length
    size_t filename_len = 0;
    while (filename[filename_len]) filename_len++;

    platform_fd_t fd = platform_path_open(filename, filename_len, rights, oflags);
    if (fd < 0) {
        return NULL;
    }

    FILE* file = alloc_file(fd);
    if (!file) {
        platform_fd_close(fd);
        return NULL;
    }

    return file;
}

int fclose(FILE *stream) {
    if (!stream || stream->fd < 0) {
        return -1;
    }

    int result = platform_fd_close(stream->fd);
    free_file(stream);
    return result;
}

int fseek(FILE *stream, long offset, int whence) {
    if (!stream || stream->fd < 0) {
        return -1;
    }

    uint64_t newoffset;
    int ret = platform_fd_seek(stream->fd, (int64_t)offset, whence, &newoffset);
    if (ret != 0) {
        stream->error = 1;
        return -1;
    }

    stream->eof = 0;
    return 0;
}

long ftell(FILE *stream) {
    if (!stream || stream->fd < 0) {
        return -1;
    }

    uint64_t offset;
    int ret = platform_fd_tell(stream->fd, &offset);
    if (ret != 0) {
        return -1;
    }
    return (long)offset;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    if (!stream || stream->fd < 0 || size == 0 || nmemb == 0) {
        return 0;
    }

    size_t total_bytes = size * nmemb;
    iovec_t iov = { .iov_base = ptr, .iov_len = total_bytes };
    size_t nread;
    int ret = platform_fd_read(stream->fd, &iov, 1, &nread);

    if (ret != 0) {
        stream->error = 1;
        return 0;
    }

    if (nread == 0) {
        stream->eof = 1;
    }

    return nread / size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    if (!stream || stream->fd < 0 || size == 0 || nmemb == 0) {
        return 0;
    }

    size_t total_bytes = size * nmemb;
    ciovec_t iov;
    iov.buf = (void *)ptr;
    iov.buf_len = total_bytes;
    size_t nwritten;
    int ret = (int)platform_fd_write(stream->fd, &iov, 1, &nwritten);

    if (ret != 0) {
        stream->error = 1;
        return 0;
    }

    return nwritten / size;
}

int fputc(int c, FILE *stream) {
    unsigned char ch = (unsigned char)c;
    if (fwrite(&ch, 1, 1, stream) != 1) {
        return EOF;
    }
    return (int)ch;
}

int fputs(const char *s, FILE *stream) {
    size_t len = 0;
    while (s[len]) len++;
    if (len == 0) return 0;
    if (fwrite(s, 1, len, stream) != len) {
        return EOF;
    }
    return 0;
}

// printf / vprintf live in printf.c; snprintf / vsnprintf live in stdlib.c.

#define FPRINTF_BUF_SIZE 4096

int vfprintf(FILE *stream, const char *format, va_list ap) {
    if (!stream || stream->fd < 0) return -1;
    char buf[FPRINTF_BUF_SIZE];
    int n = base_vsnprintf(buf, sizeof(buf), format, ap);
    if (n <= 0) return n;
    size_t to_write = (size_t)n;
    if (to_write > sizeof(buf) - 1) to_write = sizeof(buf) - 1;
    ciovec_t iov;
    iov.buf = buf;
    iov.buf_len = to_write;
    size_t nwritten;
    int ret = (int)platform_fd_write(stream->fd, &iov, 1, &nwritten);
    if (ret != 0) { stream->error = 1; return -1; }
    return n;
}

int fprintf(FILE *stream, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int r = vfprintf(stream, format, ap);
    va_end(ap);
    return r;
}

int vsprintf(char *str, const char *format, va_list ap) {
    // ISO C: write until terminating NUL; no length cap. We delegate to
    // base_vsnprintf with SIZE_MAX, which behaves like sprintf if the
    // buffer is large enough — which is the caller's responsibility.
    return base_vsnprintf(str, (size_t)-1, format, ap);
}

int sprintf(char *str, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int r = vsprintf(str, format, ap);
    va_end(ap);
    return r;
}
