#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

#if !defined(__x86_64__) && !defined(__amd64__) && !defined(__aarch64__) && !defined(_M_X64) && !defined(_M_ARM64)
    #error "System architecture not recognized, terminating program!!"
#endif

#if !defined(__linux__) && !defined(__ANDROID__) && !defined(__APPLE__) && !defined(_WIN64)
    #error "Operational system not recognized, terminating program!!"
#endif

typedef double float64;

typedef enum op_code {
    ADD, SUB, DIV, MUL, XOR, MOD, AND, SHL, SHR, POW,
    OR, LS, LSE, GR, GRE, LAND, LOR, EQ, NE, UNKNOWN
} opcode;

typedef struct operators {
    const char *op;
    int32_t precedence;
    int32_t right_assoc;
} ops;

typedef enum types {
    BC_STR, BC_FLOAT, BC_INT,
    BC_BOOL, BC_CHR, BC_NONE,
    BC_NULL
} eval_ty;

typedef char *(*S_Func)(char *operation);
typedef float64 (*F_Func)(char *operation);
typedef int64_t (*I_Func)(char *operation);

typedef struct Functions {
    const char *name;
    union func {
        I_Func i;
        F_Func f;
        S_Func s;
    } fn;
    eval_ty returnType;
    bool builtin;
} FuncEntry;

typedef struct eval_var {
    eval_ty type;
    union value {
        int64_t i;
        float64 f;
        char *s;
    } data;
} var;

extern var Ans;

typedef enum color {
    BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW,
    WHITE, GRAY, LIGHT_BLUE, LIGHT_GREEN, LIGHT_CYAN,
    LIGHT_RED, LIGHT_MAGENTA, LIGHT_YELLOW, BRIGHT_WHITE
} color4;

typedef enum paren_result {
    PAREN_OK,
    PAREN_MISSING_CLOSE,
    PAREN_MISSING_OPEN,
    PAREN_UNCLOSED_QUOTE
} paren_status;

#endif
