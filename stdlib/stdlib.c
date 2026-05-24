#include <stdint.h>
#include <stddef.h>

#include <base/mem.h>
#include <base/exit.h>
#include <base/numconv.h>
#include <platform/platform.h>
#include <base/buddy.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void* malloc(size_t size) {
    if (size == 0) {
        // ISO C says malloc(0) may return NULL or a valid pointer. We
        // return NULL because corec's buddy_alloc asserts on size==0.
        return NULL;
    }
    // Tag each block with a header recording the user-requested size,
    // so realloc() knows how much to copy. The user pointer is offset
    // past the header.
    typedef struct { size_t size; } mhdr_t;
    mhdr_t *hdr = (mhdr_t *)buddy_alloc(size + sizeof(mhdr_t), NULL);
    if (!hdr) return NULL;
    hdr->size = size;
    return (void *)(hdr + 1);
}

void free(void* ptr) {
    if (!ptr) {
        return;
    }
    typedef struct { size_t size; } mhdr_t;
    buddy_free(((mhdr_t *)ptr) - 1);
}

void *calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    if (total == 0) return NULL;
    void *p = malloc(total);
    if (p) memset(p, 0, total);
    return p;
}

void *realloc(void *ptr, size_t new_size) {
    if (!ptr) return malloc(new_size);
    if (new_size == 0) { free(ptr); return NULL; }
    typedef struct { size_t size; } mhdr_t;
    mhdr_t *hdr = ((mhdr_t *)ptr) - 1;
    size_t old_size = hdr->size;
    void *q = malloc(new_size);
    if (!q) return NULL;
    memcpy(q, ptr, old_size < new_size ? old_size : new_size);
    free(ptr);
    return q;
}

void exit(int status) {
    base_exit(status);
}

void abort(void) {
    base_abort();
}

char *getenv(const char *name) {
    // Lazily fetch the platform's environment block on the first call and
    // keep it alive for the lifetime of the program. The platform layer
    // returns the environment as a NUL-separated byte buffer plus a table
    // of pointers into that buffer; we hand callers a pointer into that
    // buffer just past the '=' separator. The buffer is owned by us, so
    // callers must treat the returned pointer as read-only and must not
    // free it (matching the contract of POSIX getenv).
    static int initialized = 0;
    static size_t env_count = 0;
    static char **env_ptrs = NULL;
    static char *env_buf = NULL;

    if (!initialized) {
        initialized = 1;
        size_t buf_size = 0;
        if (platform_environ_sizes_get(&env_count, &buf_size) != 0) {
            env_count = 0;
        } else if (env_count > 0) {
            env_ptrs = (char **)malloc(env_count * sizeof(char *));
            env_buf = (char *)malloc(buf_size);
            if (!env_ptrs || !env_buf ||
                platform_environ_get(env_ptrs, env_buf) != 0) {
                env_count = 0;
            }
        }
    }

    if (!name) return NULL;

    size_t name_len = 0;
    while (name[name_len] != '\0') {
        // Per POSIX, getenv with a name containing '=' returns NULL. This
        // also keeps us from accidentally matching Windows's hidden
        // "=DRIVE:=..." style entries (whose key is the empty string)
        // when the caller passes an empty name.
        if (name[name_len] == '=') return NULL;
        name_len++;
    }
    if (name_len == 0) return NULL;

    for (size_t i = 0; i < env_count; i++) {
        char *entry = env_ptrs[i];
        size_t k = 0;
        while (k < name_len && entry[k] != '\0' && entry[k] == name[k]) k++;
        if (k == name_len && entry[k] == '=') {
            return entry + name_len + 1;
        }
    }
    return NULL;
}

// Linear Congruential Generator (LCG)
// Parameters: a = 1103515245, c = 12345, m = 2^31
// RAND_MAX (defined in stdlib.h) = 0x7FFFFFFF.
static uint32_t rand_state = 1;

void srand(unsigned int seed) {
    rand_state = (uint32_t)seed;
}

int rand(void) {
    rand_state = (rand_state * 1103515245u + 12345u) & 0x7FFFFFFF;
    return (int)rand_state;
}

int atoi(const char* str) {
    int result = 0;
    int sign = 1;

    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n' ||
           *str == '\r' || *str == '\v' || *str == '\f') {
        str++;
    }

    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Convert digits
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return sign * result;
}

long long atoll(const char* str) {
    long long result = 0;
    int sign = 1;

    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n' ||
           *str == '\r' || *str == '\v' || *str == '\f') {
        str++;
    }

    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Convert digits
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return sign * result;
}

double atof(const char* str) {
    double result = 0.0;
    double fraction = 0.0;
    double divisor = 1.0;
    int sign = 1;
    int in_fraction = 0;

    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n' ||
           *str == '\r' || *str == '\v' || *str == '\f') {
        str++;
    }

    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Convert integer and fractional parts
    while ((*str >= '0' && *str <= '9') || *str == '.') {
        if (*str == '.') {
            in_fraction = 1;
            str++;
            continue;
        }

        if (in_fraction) {
            fraction = fraction * 10.0 + (*str - '0');
            divisor *= 10.0;
        } else {
            result = result * 10.0 + (*str - '0');
        }
        str++;
    }

    result += fraction / divisor;

    // Handle scientific notation (e or E)
    if (*str == 'e' || *str == 'E') {
        str++;
        int exp_sign = 1;
        int exponent = 0;

        if (*str == '-') {
            exp_sign = -1;
            str++;
        } else if (*str == '+') {
            str++;
        }

        while (*str >= '0' && *str <= '9') {
            exponent = exponent * 10 + (*str - '0');
            str++;
        }

        // Apply exponent
        double exp_mult = 1.0;
        for (int i = 0; i < exponent; i++) {
            exp_mult *= 10.0;
        }
        if (exp_sign < 0) {
            result /= exp_mult;
        } else {
            result *= exp_mult;
        }
    }

    return sign * result;
}

int vsnprintf(char *str, size_t size, const char *format, va_list args) {
    return base_vsnprintf(str, size, format, args);
}

int snprintf(char *str, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(str, size, format, args);
    va_end(args);
    return result;
}
