#pragma once

#ifndef INFINITY
#define INFINITY ((float)(1e308 * 1e308))
#endif

#ifndef NAN
#define NAN ((float)(INFINITY * 0.0f))
#endif

#ifndef HUGE_VAL
#define HUGE_VAL ((double)INFINITY)
#endif

double sin(double x);
double cos(double x);
double tan(double x);
double exp(double x);
double log(double x);
double pow(double x, double y);
double sqrt(double x);
double fabs(double x);
double floor(double x);
double ceil(double x);
double round(double x);
double fmod(double x, double y);

float sinf(float x);
float cosf(float x);
float tanf(float x);
float expf(float x);
float logf(float x);
float powf(float x, float y);
float sqrtf(float x);
float fabsf(float x);
float floorf(float x);
float ceilf(float x);
float roundf(float x);
float fmodf(float x, float y);
