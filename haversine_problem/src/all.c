#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <sys/stat.h>

#ifndef TYPES_
#define TYPES_

typedef enum {false, true} bool;
typedef double f64;
typedef long i64;
typedef char u8;
typedef unsigned int u32;
typedef uint64_t u64;

#endif

#define ArraySize(arr) (sizeof(arr) / sizeof(*arr))
