#ifndef UTILS_H
#define UTILS_H

#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _CRT_NONSTDC_NO_WARNINGS
    #define _CRT_NONSTDC_NO_WARNINGS
#endif

#ifndef _POSIX_C_SOURCE
    #ifdef __APPLE__
        #define _POSIX_C_SOURCE 200809L
    #else
        #define _POSIX_C_SOURCE 199309L
    #endif
#endif

#include <time.h>
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>


#ifdef _WIN64
    #include <io.h>
    #include <windows.h>

    #define _isatty isatty
    #define STDIN_FILENO 0
    #define strcasecmp _stricmp
    #define strncasecmp _strnicmp
    #define strdup _strdup

    extern HANDLE hConsole;
#else

    #ifndef CLOCK_REALTIME
        #define CLOCK_REALTIME 0
    #endif

    #include <unistd.h>
    #include <stddef.h>
    #include <strings.h>
#endif

#define VERSION "r0.2.90"

#define INIT_MESSAGE \
    "ceval " VERSION"\na simple eval function implemented in C, \n" \
    "type 'mathlib' if you're insite ceval to turn on the math library, " \
    "'cls' or 'clear' to clear the screen and scrollback buffer and 'help' to display the manual.\n"

#define DBL_PRECISION 10

#define E_VAR "E"
#define PI_VAR "PI"
#define ANS_VAR "ans"
#define INT_VAR "int"
#define INF_VAR "inf"
#define STR_VAR "str"
#define CHR_VAR "chr"
#define NONE_VAR "none"
#define NULL_VAR "NULL"
#define TRUE_VAR "true"
#define BOOL_VAR "bool"
#define FLOAT_VAR "float"
#define FALSE_VAR "false"
#define RAND_MAX_VAR "rand_max"

#define SINGLE_QUOTES 0
#define DOUBLE_QUOTES 1
#define BOTH_QUOTES   2

#define HEX_PREF "0x"
#define OCT_PREF "0o"
#define BIN_PREF "0b"

#define EPS 1e-9

#define BC_PROMPT_COLOR 0x0D
#define GET_BASE_COLOR(color) (((color) < (LIGHT_BLUE)) ? (color) : (color) - 0x08)

#define T_CMP(num1, num2) (fabs((num1) - (num2)) < (EPS))

#define MAX_CHAR 0x1000
#define INPUT_SIZE MAX_CHAR+5

#define I64_NAN ((int64_t)(INT64_MAX - 1ULL))
#define U32_NAN ((uint32_t)(UINT32_MAX - 1U))
#define MAX_SAFE_INT64_D  9223372036854775807.0
#define MIN_SAFE_INT64_D -9223372036854775808.0
#define U64_NAN ((uint64_t)(UINT64_MAX - 1ULL))

#define SAFE_FREE(ptr) do { \
    if (ptr) { \
        free(ptr); \
        ptr = NULL; \
    } \
} while (false)

#define alloc(bytes) safe_malloc(bytes, __LINE__, __FILE__)

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
opcode get_opcode(const char *op);
void extractParenthesis(char *str);
void charRm(char *str, int8_t targ);
eval_ty eval_typeof(const char *str);
bool isValidBcFuncName(const char *str);
bool trim_streq(char *str1, char *str2);
void shiftLeft_at(char *str, size_t pos);
int32_t bc_strcmp(char *str1, char *str2);
int16_t strchar(const char *str, int8_t chr);
bool is_wrapped_by_parentheses(const char *s);
paren_status parenthesis_check(const char *str);
char *bc_strcat(const char *dest, const char *src);
void num_snprintf(char *buff, size_t size, var num);
bool isQuoted(const char *string, int16_t quoteMode);
bool isBcVariable(const char *str, bool *shouldError);
void getItemTypeStr(char *buff, size_t size, var item);
int16_t injectEscape(char *str, const char *error_str);
void getItemTypeStr(char *buff, size_t size, var item);
void int64_to_hex_min(int64_t v, char *out, size_t size);
void *safe_malloc(size_t bytes, uint32_t LINE, char *FILE);
int8_t getInvalidEscape(const char *str, const char *error_str);
void printc(const char *fmt, color4 initColor, color4 resetColor, ...);
int16_t find_main_operator_full(const char *s, const ops *operators, char *foundOp);

#endif
