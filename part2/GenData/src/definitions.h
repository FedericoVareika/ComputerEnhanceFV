#pragma once

typedef double f64;
typedef unsigned int u32;

typedef enum { false, true } bool;

typedef enum { none, cluster, uniform } Method;

#define ArraySize(arr) (sizeof(arr) / sizeof(*arr))
