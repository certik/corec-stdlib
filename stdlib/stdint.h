#pragma once

// <stdint.h> — fixed-width integer types and their limits.
//
// The types (int8_t … uint64_t, uintptr_t, size_t, SIZE_MAX,
// INTNN_C / UINTNN_C) come from corec/base/types.h, which also defines
// a few of the limits (INT32_MAX, UINT16_MAX, UINT32_MAX, INT64_MAX,
// UINT64_MAX). The remaining ISO-mandated limits are defined here for
// symmetry, so user code can portably use any INTn_MIN/MAX.

#include <base/types.h>

#define INT8_MIN  (-128)
#define INT8_MAX  127
#define UINT8_MAX 255

#define INT16_MIN  (-32768)
#define INT16_MAX  32767
// UINT16_MAX comes from <base/types.h>.

#define INT32_MIN  (-INT32_MAX - 1)
// INT32_MAX, UINT32_MAX come from <base/types.h>.

#define INT64_MIN  (-INT64_MAX - 1)
// INT64_MAX, UINT64_MAX come from <base/types.h>.
