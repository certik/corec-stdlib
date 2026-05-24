#pragma once

#include <stdint.h>
#include <stddef.h>

// NULL — re-exposed here in addition to <stddef.h> for compatibility
// with code that includes <stdlib.h> alone.
#ifndef NULL
#define NULL ((void*)0)
#endif

// exit() status macros.
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

// Maximum value returned by rand(). Our rand() uses a 31-bit LCG.
#define RAND_MAX 0x7FFFFFFF

// Memory management — currently backed by corec's buddy allocator.
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void  free(void *ptr);

// Process control.
void exit(int status);
void abort(void);

// Environment access. The wasm32-wasi backend doesn't expose an
// environment to the module, so this currently always returns NULL.
char *getenv(const char *name);

// Pseudo-random numbers (linear congruential, deterministic for a given seed).
void srand(unsigned int seed);
int  rand(void);

// String → number conversions.
int       atoi(const char *str);
long long atoll(const char *str);
double    atof(const char *str);
