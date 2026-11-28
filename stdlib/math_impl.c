#include "math.h"
#include <base/math.h>

double sin(double x) { return fast_sin(x); }
double cos(double x) { return fast_cos(x); }
double tan(double x) { return fast_tan(x); }
double exp(double x) { return fast_exp(x); }
double log(double x) { return fast_log(x); }
double pow(double x, double y) { return fast_pow(x, y); }
double sqrt(double x) { return fast_sqrt(x); }
double fabs(double x) { return base_fabs(x); }
double floor(double x) { return base_floor(x); }
double ceil(double x) { return base_ceil(x); }
double round(double x) { return base_round(x); }
double fmod(double x, double y) { return base_fmod(x, y); }

float sinf(float x) { return fast_sinf(x); }
float cosf(float x) { return fast_cosf(x); }
float tanf(float x) { return fast_tanf(x); }
float expf(float x) { return fast_expf(x); }
float logf(float x) { return fast_logf(x); }
float powf(float x, float y) { return fast_powf(x, y); }
float sqrtf(float x) { return fast_sqrtf(x); }
float fabsf(float x) { return base_fabsf(x); }
float floorf(float x) { return base_floorf(x); }
float ceilf(float x) { return base_ceilf(x); }
float roundf(float x) { return base_roundf(x); }
float fmodf(float x, float y) { return base_fmodf(x, y); }
