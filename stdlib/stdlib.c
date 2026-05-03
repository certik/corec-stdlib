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
    return buddy_alloc(size, NULL);
}

void free(void* ptr) {
    if (!ptr) {
        return;
    }
    buddy_free(ptr);
}

void exit(int status) {
    base_exit(status);
}

void abort(void) {
    base_abort();
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
