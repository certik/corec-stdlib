#pragma once

#include <base/types.h>

#ifndef offsetof
#define offsetof(type, member) __builtin_offsetof(type, member)
#endif

// Re-export types from base/types.h for stdlib compatibility
