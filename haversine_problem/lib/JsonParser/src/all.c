#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#include "buffer.c"
#include "structures.c"
#include "parse_json.c"
#include "pretty_printer.c"
#include "parse_values.c"
