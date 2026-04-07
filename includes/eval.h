#ifndef EVAL_H
#define EVAL_H

#include "types.h"
#include <stdbool.h>

#ifndef _WIN64
typedef unsigned long size_t;
#endif

#ifdef _WIN64
    #define clear(void) system("cls")
#else
    #define clear(void) system("clear")
#endif

char *var2str(var buff);
var eval(char *operation, bool mathlib);
var calc(var left, const char *operation, var right, bool mathLib);
var parse_operation(char *operation, const FuncEntry *functions, size_t funcCount, const ops *operator, bool mathlib);

#endif
