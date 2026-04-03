#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include <time.h>
#include "types.h"
#include <ctype.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include "s_math.h"
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <inttypes.h>

#ifdef _WIN64
    #include <io.h>
    #include <windows.h>
    #define _isatty isatty
    extern HANDLE hConsole;
#else
    #include <unistd.h>
#endif

#define E_VAR "E"
#define PI_VAR "PI"
#define ANS_VAR "ans"
#define INT_VAR "int"
#define INF_VAR "inf"
#define STR_VAR "str"
#define CHR_VAR "chr"
#define NONE_VAR "none"
#define TRUE_VAR "true"
#define BOOL_VAR "bool"
#define FLOAT_VAR "float"
#define FALSE_VAR "false"
#define RAND_MAX_VAR "rand_max"

#define HEX_PREF "0x"
#define OCT_PREF "0o"
#define BIN_PREF "0b"

#define EPS 1e-9

#define BC_PROMPT_COLOR 0x0D
#define GET_BASE_COLOR(color) (((color) < (LIGHT_BLUE)) ? (color) : (color) - 0x08)

#define T_CMP(num1, num2) (fabs((num1) - (num2)) < (EPS))

#define MAX_CHAR 0x400

#define I64_NAN ((int64_t)(INT64_MAX - 1ULL))
#define U32_NAN ((uint32_t)(UINT32_MAX - 1U))
#define MAX_SAFE_INT64_D  9223372036854775807.0
#define MIN_SAFE_INT64_D -9223372036854775808.0
#define U64_NAN ((uint64_t)(UINT64_MAX - 1ULL))


void trim(char *str);
void initRandom(void);
void trimEnd(char *str);
void print_manual(void);
bool isBin(const char *str);
void setColor(color4 color);
bool isHex(const char *str);
bool isOct(const char *str);
int64_t hex_to_long(char *str);
bool isalldigit(const char *s);
void removeComments(char *str);
void extractParenthesis(char *str);
void charRm(char *str, int8_t targ);
bool isIn(char needle, char *haystack);
bool isValidBcFuncName(const char *str);
void shiftLeft_at(char *str, size_t pos);
int32_t bc_strcmp(char *str1, char *str2);
int16_t strchar(const char *str, int8_t chr);
bool is_wrapped_by_parentheses(const char *s);
paren_status parenthesis_check(const char *str);
bool isValidBcCommand(char *str, char *command);
char *bc_strcat(const char *dest, const char *src);
bool isBcVariable(const char *str, bool *shouldError);
void getItemTypeStr(char *buff, size_t size, var item);
int16_t injectEscape(char *str, const char *error_str);
double parse_base_fraction(const char *s, int8_t base);
void getItemTypeStr(char *buff, size_t size, var item);
void num_snprintf(char *buff, size_t size, double num);
void int64_to_hex_min(int64_t v, char *out, size_t size);
bool isBetweenQuotes(const char *action, int16_t quoteMode);
double parse_bin_hex_oct_ans_e_pi(const char *str, int16_t *ok);
int8_t getInvalidEscape(const char *str, const char *error_str);
void printc(const char *str, color4 initColor, color4 resetColor, ...);
int16_t find_main_operator_full(const char *s, const char **multiOps, const char *uniOps, char *foundOp);

#define SAFE_FREE(ptr) do { \
    if (ptr) { \
        free(ptr); \
        ptr = NULL; \
    } \
} while (false)

#endif