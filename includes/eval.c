#define _GNU_SOURCE

#include "eval.h"
#include "types.h"
#include <math.h>
#include "utils.h"
#include <ctype.h>
#include "s_math.h"


static int32_t eval_depth = 0;
var Ans = { .type = BC_NULL };

var eval(char *operation, bool mathlib) {

    if (!operation)
        return (var){.type = BC_NULL};

    const FuncEntry math_table[] = {
    {.returnType = BC_NONE,   .name = "print",     .fn.i = s_print,       .builtin = true},
    {.returnType = BC_BOOL,   .name = "isprime",   .fn.i = s_isprime,     .builtin = false},
    {.returnType = BC_BOOL,   .name = BOOL_VAR,    .fn.i = bc_bool,       .builtin = true},
    {.returnType = BC_INT,    .name = "scale",     .fn.i = s_scale,       .builtin = false},
    {.returnType = BC_INT,    .name = "floor",     .fn.i = s_floor,       .builtin = true},
    {.returnType = BC_INT,    .name = "ceil",      .fn.i = s_ceil,        .builtin = true},
    {.returnType = BC_INT,    .name = "round",     .fn.i = s_round,       .builtin = true},
    {.returnType = BC_INT,    .name = "trunc",     .fn.i = s_trunc,       .builtin = false},
    {.returnType = BC_INT,    .name = "rand",      .fn.i = s_randInt,     .builtin = false},
    {.returnType = BC_INT,    .name = "len",       .fn.i = bc_len,        .builtin = true},
    {.returnType = BC_INT,    .name = INT_VAR,     .fn.i = bc_int,        .builtin = true},
    {.returnType = BC_FLOAT,  .name = "sqrt",      .fn.f = s_sqrt,        .builtin = false},
    {.returnType = BC_FLOAT,  .name = "tet",       .fn.f = s_tet,         .builtin = false},
    {.returnType = BC_FLOAT,  .name = "pow",       .fn.f = s_pow,         .builtin = false},
    {.returnType = BC_FLOAT,  .name = "root",      .fn.f = s_root,        .builtin = false},
    {.returnType = BC_FLOAT,  .name = "sin",       .fn.f = s_sin,         .builtin = false},
    {.returnType = BC_FLOAT,  .name = "asin",      .fn.f = s_asin,        .builtin = false},
    {.returnType = BC_FLOAT,  .name = "cos",       .fn.f = s_cos,         .builtin = false},
    {.returnType = BC_FLOAT,  .name = "acos",      .fn.f = s_acos,        .builtin = false},
    {.returnType = BC_FLOAT,  .name = "tan",       .fn.f = s_tan,         .builtin = false},
    {.returnType = BC_FLOAT,  .name = "atan",      .fn.f = s_atan,        .builtin = false},
    {.returnType = BC_FLOAT,  .name = "cot",       .fn.f = s_cot,         .builtin = false},
    {.returnType = BC_FLOAT,  .name = "acot",      .fn.f = s_acot,        .builtin = false},
    {.returnType = BC_FLOAT,  .name = "ln",        .fn.f = s_ln,          .builtin = false},
    {.returnType = BC_FLOAT,  .name = "log10",     .fn.f = s_log10,       .builtin = false},
    {.returnType = BC_FLOAT,  .name = "log2",      .fn.f = s_log2,        .builtin = false},
    {.returnType = BC_FLOAT,  .name = "log",       .fn.f = s_log,         .builtin = false},
    {.returnType = BC_FLOAT,  .name = "sign",      .fn.f = s_sign,        .builtin = true},
    {.returnType = BC_FLOAT,  .name = "sum",       .fn.f = s_sum,         .builtin = false},
    {.returnType = BC_FLOAT,  .name = "deg2rad",   .fn.f = s_rad,         .builtin = false},
    {.returnType = BC_FLOAT,  .name = "rad2gon",   .fn.f = s_gon,         .builtin = false},
    {.returnType = BC_FLOAT,  .name = "rad2deg",   .fn.f = s_deg,         .builtin = false},
    {.returnType = BC_FLOAT,  .name = "randf",     .fn.f = s_randFloat,   .builtin = false},
    {.returnType = BC_FLOAT,  .name = "fah",       .fn.f = s_fah,         .builtin = false},
    {.returnType = BC_FLOAT,  .name = "cel",       .fn.f = s_cel,         .builtin = false},
    {.returnType = BC_FLOAT,  .name = "mi",        .fn.f = s_miles,       .builtin = false},
    {.returnType = BC_FLOAT,  .name = "km",        .fn.f = s_km,          .builtin = false},
    {.returnType = BC_FLOAT,  .name = "lb",        .fn.f = s_pounds,      .builtin = false},
    {.returnType = BC_FLOAT,  .name = "kg",        .fn.f = s_kg,          .builtin = false},
    {.returnType = BC_FLOAT,  .name = "bmi",       .fn.f = s_bmi,         .builtin = false},
    {.returnType = BC_FLOAT,  .name = "feet",      .fn.f = s_feet,        .builtin = false},
    {.returnType = BC_FLOAT,  .name = "meter",     .fn.f = s_meter,       .builtin = false},
    {.returnType = BC_FLOAT,  .name = "abs",       .fn.f = s_abs,         .builtin = true},
    {.returnType = BC_FLOAT,  .name = FLOAT_VAR,   .fn.f = bc_float,      .builtin = true},
    {.returnType = BC_STR,    .name = STR_VAR,     .fn.s = bc_parse_str,  .builtin = true},
    {.returnType = BC_STR,    .name = CHR_VAR,     .fn.s = s_chr,         .builtin = true},
    {.returnType = BC_STR,    .name = "bin",       .fn.s = s_bin,         .builtin = true},
    {.returnType = BC_STR,    .name = "oct",       .fn.s = s_oct,         .builtin = true},
    {.returnType = BC_STR,    .name = "hex",       .fn.s = s_hex,         .builtin = true},
    {.returnType = BC_STR,    .name = "lower",     .fn.s = s_lower,       .builtin = false},
    {.returnType = BC_STR,    .name = "upper",     .fn.s = s_upper,       .builtin = false},
    {.returnType = BC_STR,    .name = "typeof",    .fn.s = bc_typeof,     .builtin = true},
    {.returnType = BC_STR,    .name = "input",     .fn.s = s_input,       .builtin = true},
    };

    size_t funcCount = sizeof(math_table) / sizeof(*math_table);

    const ops operators[] = {
        {.op = "**",  .precedence = 7,  .right_assoc = 1},
        {.op = "<<",  .precedence = 4,  .right_assoc = 0},
        {.op = ">>",  .precedence = 4,  .right_assoc = 0},
        {.op = "<=",  .precedence = 3,  .right_assoc = 0},
        {.op = ">=",  .precedence = 3,  .right_assoc = 0},
        {.op = "==",  .precedence = 2,  .right_assoc = 0},
        {.op = "!=",  .precedence = 2,  .right_assoc = 0},
        {.op = "&&",  .precedence = -2, .right_assoc = 0},
        {.op = "||",  .precedence = -3, .right_assoc = 0},

        {.op = "*",   .precedence = 6,  .right_assoc = 0}, 
        {.op = "/",   .precedence = 6,  .right_assoc = 0},
        {.op = "%",   .precedence = 6,  .right_assoc = 0},
        {.op = "+",   .precedence = 5,  .right_assoc = 0},
        {.op = "-",   .precedence = 5,  .right_assoc = 0},
        {.op = "<",   .precedence = 3,  .right_assoc = 0},
        {.op = ">",   .precedence = 3,  .right_assoc = 0},
        {.op = "&",   .precedence = 1,  .right_assoc = 0},
        {.op = "^",   .precedence = 0,  .right_assoc = 0},
        {.op = "|",   .precedence = -1, .right_assoc = 0},

        {.op = NULL,  .precedence = 0,  .right_assoc = 0}
    };

    paren_status status = parenthesis_check(operation);

    if (status != PAREN_OK) {

        switch (status) {
            case PAREN_MISSING_CLOSE:
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("expected ')'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

                break;

            case PAREN_MISSING_OPEN:
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("unexpected ')'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

                break;

            case PAREN_UNCLOSED_QUOTE:
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("unclosed quote\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

                break;

            default:
                break;
        }

        return (var){.type = BC_NULL};
    }

    for (size_t i = 0; operation[i]; i++) {
        if ((unsigned char)operation[i] > 0x80) {
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("cannot work with multi-byte characters\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
    
            return (var){.type = BC_NULL};
        }
    }


    char tmp[MAX_CHAR] = {0};
    size_t len = strlen(operation);
    memcpy(tmp, operation, len + 1);

    removeComments(tmp);
    trim(tmp);
    trimEnd(tmp);

    if (!*tmp)
        return (var){.type = BC_NULL};

    if (!getInvalidEscape(tmp))
        return (var){.type = BC_NULL};

    eval_depth++;

    var buff = parse_operation(tmp, math_table, funcCount, operators, mathlib);

    if (mathlib) {
        switch (buff.type) {
            case BC_NULL:
                break;

            case BC_STR:
                if (eval_depth == 1) {
                    if (Ans.type == BC_STR && Ans.data.s)
                        SAFE_FREE(Ans.data.s);

                    Ans.type = BC_STR;
                    Ans.data.s = strdup(buff.data.s);
                }

                break;

            default:
                if (eval_depth == 1)
                    Ans = buff;
        }
    }

    eval_depth--;
    return buff;
}

char *var2str(var buff) {
    switch (buff.type) {

        case BC_NONE:
            return strdup(NONE_VAR);

        case BC_STR:
            return buff.data.s;

        case BC_BOOL:
            return (buff.data.i == false) ? strdup(FALSE_VAR) : strdup(TRUE_VAR);

        case BC_CHR:
        case BC_INT:
        case BC_FLOAT: {
            const size_t max = 0x60;
            char *tmp = alloc(max);
            if (!tmp) return NULL;

            num_snprintf(tmp, max, buff);
            return tmp;
        }

        default:
            return NULL;
    }
}

var calc(var left, const char *operation, var right, bool mathLib) {

    var out;
    out.type = BC_NULL;

    if (left.type == BC_NULL || right.type == BC_NULL) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("invalid data type: '"NULL_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
        return out;
    }

    opcode code = get_opcode(operation);

    if ((left.type == BC_NONE || right.type == BC_NONE) && code != EQ && code != NE) {
        char type1[0x10] = {0};
        getItemTypeStr(type1, sizeof(type1), left);

        char type2[0x10] = {0};
        getItemTypeStr(type2, sizeof(type2), right);

        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("bad operand combination with '%s' and '%s' with operation: '%s'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, type1, type2, operation);

        return out;
    }

    if (left.type == BC_STR || right.type == BC_STR) {

        switch (code) {
            case ADD: {
                if (left.type != right.type) {
                    char type[0x20] = {0};
                    var wrong = (left.type != BC_STR) ? left : right;

                    getItemTypeStr(type, sizeof(type), wrong);

                    printc("ceval", BC_PROMPT_COLOR, WHITE);
                    printf(": ");
                    printc("cannot concatenate strings with '%s' type\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, type);

                    return out;
                }

                if (strlen(left.data.s) < 2 || strlen(right.data.s) < 2) {
                    printc("ceval", BC_PROMPT_COLOR, WHITE);
                    printf(": ");
                    printc("invalid string format\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                    return out;
                }

                out.type = BC_STR;
                out.data.s = bc_strcat(left.data.s, right.data.s);

                if (!out.data.s) {
                    out.type = BC_NULL;
                    return out;
                }

                return out;
            }

            case MUL: {
                var notStr;
                var Str;

                if (left.type != BC_STR) {
                    notStr = left;
                    Str = right;
                } else {
                    notStr = right;
                    Str = left;
                }

                if (notStr.type != BC_INT && notStr.type != BC_CHR) {
                    char type[0x20] = {0};

                    getItemTypeStr(type, sizeof(type), notStr);

                    printc("ceval", BC_PROMPT_COLOR, WHITE);
                    printf(": ");
                    printc("cannot multiply strings with type '%s'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, type);

                    return out;
                }

                int64_t multiplier = notStr.data.i; 
                char *multiplied_str = Str.data.s;

                if (multiplier <= 0) {
                    printc("ceval", BC_PROMPT_COLOR, WHITE);
                    printf(": ");
                    printc("to multiply strings the multiplier must be at least greater than 0\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                    return out;
                }

                if (((long)strlen(multiplied_str) - 2) * (size_t)multiplier > MAX_CHAR) {
                    printc("ceval", BC_PROMPT_COLOR, WHITE);
                    printf(": ");
                    printc("the resultant string must be less than %d characters long\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, MAX_CHAR);
                    return out;
                }

                out.type = BC_STR;
                char *result = strdup(multiplied_str);
                if (!result) {
                    out.type = BC_NULL;
                    return out;
                }

                for (size_t i = 1; i < (size_t)multiplier; i++) {
                    char *old = result;
                    result = bc_strcat(result, multiplied_str);

                    SAFE_FREE(old);
                    if (!result) { 
                        out.type = BC_NULL;
                        return out;
                    }
                }

                out.data.s = result;
                return out;
            }

            case EQ: {
                out.type = BC_BOOL;

                if (left.type != right.type) {
                    out.data.i = false;
                    return out;
                }

                out.data.i = bc_strcmp(left.data.s, right.data.s) == 0;
                return out;
            }

            case NE: {
                out.type = BC_BOOL;

                if (left.type != right.type) {
                    out.data.i = true;
                    return out;
                }

                out.data.i = bc_strcmp(left.data.s, right.data.s) != 0;
                return out;
            }

            case GR: {
                out.type = BC_BOOL;

                if (left.type != right.type) {
                    out.data.i = false;
                    return out;
                }

                out.data.i = bc_strcmp(left.data.s, right.data.s) > 0;
                return out;
            }

            case GRE: {
                out.type = BC_BOOL;

                if (left.type != right.type) {
                    out.data.i = false;
                    return out;
                }

                out.data.i = bc_strcmp(left.data.s, right.data.s) >= 0;
                return out;
            }

            case LS: {
                out.type = BC_BOOL;

                if (left.type != right.type) {
                    out.data.i = false;
                    return out;
                }

                out.data.i = bc_strcmp(left.data.s, right.data.s) < 0;
                return out;
            }

            case LSE: {
                out.type = BC_BOOL;

                if (left.type != right.type) {
                    out.data.i = false;
                    return out;
                }

                out.data.i = bc_strcmp(left.data.s, right.data.s) <= 0;
                return out;
            }

            default: {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("unsupported operand for '"STR_VAR"' type: '%s'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, operation);

                return out;
            }
        }
    }

    float64 num1;
    float64 num2;

    switch (left.type) {
        case BC_CHR:
        case BC_BOOL:
        case BC_INT:
            if (left.type == BC_BOOL)
                left.type = BC_INT;

            num1 = (float64)left.data.i;
            break;
        case BC_FLOAT:
            num1 = left.data.f;
            break;
        case BC_NONE:
            break;
        default:
            return out;
    }

    switch (right.type) {
        case BC_CHR:
        case BC_BOOL:
        case BC_INT:
            if (right.type == BC_BOOL)
                right.type = BC_INT;

            num2 = (float64)right.data.i;
            break;
        case BC_FLOAT:
            num2 = right.data.f;
            break;
        case BC_NONE:
            break;
        default:
            return out;
    }

    float64 result = 0;

    if (left.type == BC_FLOAT || right.type == BC_FLOAT)
        out.type = BC_FLOAT;

    switch (code) {
        case ADD:
            result = num1 + num2;
            break;
        case SUB:
            result = num1 - num2;
            break;
        case DIV:
            if (num2 == 0.0) {

                if (!mathLib) {
                    printc("ceval", BC_PROMPT_COLOR, WHITE);
                    printf(": ");
                    printc("cannot divide by 0\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                    out.type = BC_NULL;
                    return out;
                }

                int32_t negative = signbit(num1) ^ signbit(num2);
                result = negative ? -INFINITY : INFINITY;

            } else
                result = num1 / num2;

            break;

        case MUL:
            result = num1 * num2;
            break;

        case MOD:
            if (num2 == 0) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("cannot divide by 0\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                out.type = BC_NULL;
                return out;
            }

            result = fmod(num1, num2);
            break;

        case XOR:
            if ((left.type != BC_INT && left.type != BC_CHR) || (right.type != BC_INT && right.type != BC_CHR)) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("'^' requires type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                out.type = BC_NULL;
                return out;
            }

            result = (int64_t)num1 ^ (int64_t)num2;

            break;

        case AND:
            if ((left.type != BC_INT && left.type != BC_CHR) || (right.type != BC_INT && right.type != BC_CHR)) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("'&' requires type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                out.type = BC_NULL;
                return out;
            }

            result = (int64_t)num1 & (int64_t)num2;
            break;

        case OR:
            if (left.type == BC_FLOAT || right.type == BC_FLOAT) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("'|' requires type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                out.type = BC_NULL;
                return out;
            }

            result = (int64_t)num1 | (int64_t)num2;
            break;

        case LS:
            out.type = BC_BOOL;
            out.data.i = (num1 < num2) && !T_CMP(num1, num2);
            break;

        case LSE:
            out.type = BC_BOOL;
            out.data.i = (num1 < num2) || T_CMP(num1, num2);
            break;

        case GR:
            out.type = BC_BOOL;
            out.data.i = (num1 > num2) && !T_CMP(num1, num2);
            break;

        case GRE:
            out.type = BC_BOOL;
            out.data.i = (num1 > num2) || T_CMP(num1, num2);
            break;

        case NE:
            out.type = BC_BOOL;

            if (left.type == BC_NONE || right.type == BC_NONE) {
                if (left.type == BC_NONE && right.type == BC_NONE)
                    out.data.i = false;
                else
                    out.data.i = true;

                break;
            }

            if (isnan(num1) || isnan(num2))
                out.data.i = true;

            else if (isinf(num1) || isinf(num2))
                out.data.i = (num1 != num2);

            else
                out.data.i = !T_CMP(num1, num2);

            break;

        case EQ:
            out.type = BC_BOOL;

            if (left.type == BC_NONE || right.type == BC_NONE) {
                if (left.type == BC_NONE && right.type == BC_NONE)
                    out.data.i = true;
                else
                    out.data.i = false;

                break;
            }

            if (isnan(num1) || isnan(num2))
                out.data.i = false;

            else if (isinf(num1) || isinf(num2))
                out.data.i = (num1 == num2);

            else
                out.data.i = T_CMP(num1, num2);

            break;

        case SHL:
            if (left.type == BC_FLOAT || right.type == BC_FLOAT) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("'<<' requires type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                out.type = BC_NULL;
                return out;
            }

            if (num2 < 0 || num2 >= sizeof(uint64_t) * 8) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("invalid shift amount\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                out.type = BC_NULL;
                return out;
            }

            result = (uint64_t)num1 << (uint64_t)num2;
            break;

        case SHR:
            if (left.type == BC_FLOAT || right.type == BC_FLOAT) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("'>>' requires type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                out.type = BC_NULL;
                return out;
            }

            if (num2 < 0 || num2 >= sizeof(int64_t) * 8) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("invalid shift amount\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                out.type = BC_NULL;
                return out;
            }

            result = (int64_t)num1 >> (int64_t)num2;
            break;

        case LAND:
            out.type = BC_BOOL;
            out.data.i = (num1 != 0 && num2 != 0);
            break;

        case LOR:
            out.type = BC_BOOL;
            out.data.i = (num1 != 0 || num2 != 0);
            break;

        case POW:
            if (num1 < 0 && right.type == BC_FLOAT) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("to use negative bases the power must not be of type '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

                out.type = BC_NULL;
                return out;
            }

            result = pow(num1, num2);
            break;

        default:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("Unknown operator '%s'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, operation);
            out.type = BC_NULL;
            return out;
    }

    if (out.type == BC_BOOL)
        return out;
    else if (out.type == BC_FLOAT) {
        out.data.f = result;
        return out;
    }

    if (T_CMP(result, (int64_t)result)) {
        out.type = BC_INT;
        out.data.i = (int64_t)result;
    } else {
        out.type = BC_FLOAT;
        out.data.f = result;
    }

    return out;
}

static var lNot_section(char *operation, bool mathlib) {
    size_t count = 0;
    while (operation[count] == '!')
        count++;

    char *expr = operation + count;

    if (!*expr) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("syntax error\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return (var){ .type = BC_NULL };
    }

    if (isQuoted(expr, DOUBLE_QUOTES)) {
        if (count & 1)
            return (var){ .type = BC_BOOL, .data.i = (strlen(expr) > 2) ? false : true};
        else
            return (var){ .type = BC_BOOL, .data.i = (strlen(expr) > 2) ? true : false};
    }

    if (mathlib) {
        if (Ans.type == BC_STR && strcmp(expr, ANS_VAR) == 0) {
            if (!Ans.data.s)
                return (var){ .type = BC_BOOL, .data.i = false };

            if (count & 1)
                return (var){ .type = BC_BOOL, .data.i = false };
            else
                return (var){ .type = BC_BOOL, .data.i = true };
        }
    }

    var tmp = eval(expr, mathlib);

    if (tmp.type == BC_NULL)
        return (var){.type = BC_NULL};

    float64 num = (tmp.type == BC_BOOL) ? (float64)tmp.data.i : tmp.data.f;
    if (tmp.type == BC_INT) {
        int64_t val = tmp.data.i;
        bool value = (val != 0.0);

        if (count & 1)
            value = !value;

        return (var){ .type = BC_BOOL, .data.i = value };
    }

    if (isnan(num))
        return (var){ .type = BC_NULL };

    bool value = (num != 0.0);

    if (count & 1)
        value = !value;

    return (var){ .type = BC_BOOL, .data.i = value };
}

static var func_section(char *operation, size_t funcCount, const FuncEntry *functions, bool mathlib) {
    char name[0x100] = {0};

    long parenthesis_index = strchar(operation, '(');
    if (parenthesis_index == -1)
        return h_atof(operation, mathlib);

    memcpy(name, operation, parenthesis_index);
    name[parenthesis_index] = '\0';

    trimEnd(name);

    if (!*name)
        return (var){ .type = BC_NULL };

    if (*operation == '~' || *operation == '-') {
        var tmp = h_atof(operation, mathlib);

        if (tmp.type == BC_FLOAT && isnan(tmp.data.f))
            return (var){ .type = BC_NULL };

        return tmp;
    }

    int32_t depth = 0;
    int32_t close_index = -1;

    for (int32_t i = parenthesis_index; operation[i]; i++) {
        if (operation[i] == '(')
            depth++;
        else if (operation[i] == ')') {
            depth--;
            if (depth == 0) {
                close_index = i;
                break;
            }
        }
    }

    if (close_index == -1 || operation[close_index + 1] != '\0') {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("invalid syntax\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return (var){ .type = BC_NULL };
    }

    if (!isValidBcFuncName(name)) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("invalid function name: '%s()'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, name);

        return (var){ .type = BC_NULL };
    }

    for (size_t i = 0; i < funcCount; i++) {
        if (strcmp(name, functions[i].name) != 0)
            continue;

        if (!functions[i].builtin && !mathlib)
            continue;

        switch (functions[i].returnType) {
            case BC_BOOL:
            case BC_INT: {
                int64_t num = functions[i].fn.i(operation);

                if (num == I64_NAN)
                    return (var){ .type = BC_NULL };

                if (functions[i].returnType == BC_BOOL)
                    return (var){ .type = BC_BOOL, .data.i = (bool)num };

                return (var){ .type = functions[i].returnType, .data.i = num };
            }

            case BC_FLOAT: {
                float64 num = functions[i].fn.f(operation);

                if (isnan(num))
                    return (var){ .type = BC_NULL };

                return (var){ .type = functions[i].returnType, .data.f = num };
            }

            case BC_STR: {
                char *result = functions[i].fn.s(operation);

                if (!result)
                    return (var){ .type = BC_NULL };

                return (var){ .type = functions[i].returnType, .data.s = result};
            }

            case BC_NONE: {
                bool status = functions[i].fn.i(operation);

                if (!status)
                    return (var){ .type = BC_NULL };

                return (var){ .type = BC_NONE };
            }

            default: {
                char type[0x14] = {0};

                getItemTypeStr(type, sizeof(type), (var){.type = functions[i].returnType});

                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("invalid function with '%s' unknown type: '%s()'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, type, name);

                return (var){ .type = BC_NULL} ;
            }
        }
    }

    printc("ceval", BC_PROMPT_COLOR, WHITE);
    printf(": ");
    printc("undefined function: '%s()'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, name);

    return (var){ .type = BC_NULL };
}

static var parse_single(char *operation, const FuncEntry *functions, size_t funcCount, bool mathlib) {
    if (mathlib && Ans.type == BC_STR && strcmp(operation, ANS_VAR) == 0)
        return (var){ .type = BC_STR, .data.s = strdup(Ans.data.s)};

    if (*operation == '!')
        return lNot_section(operation, mathlib);

    char *paren = strchr(operation, '(');

    if (!paren) {

        if (strcmp(operation, TRUE_VAR) == 0)
            return (var){ .type = BC_BOOL, .data.i = true };
        else if (strcmp(operation, FALSE_VAR) == 0)
            return (var){ .type = BC_BOOL, .data.i = false };

        size_t len = strlen(operation);

        if (len > 1 && *operation == '"') {
            char result[0x400] = {0};
            size_t res_len = 0;
            const char *p = operation;

            while (*p) {
                while (isspace((unsigned char)*p))
                    p++;

                if (*p != '"')
                    break;

                p++;

                while (*p) {

                    if (*p == '"' && !is_escaped(operation, p - operation))
                        break;

                    if (*p == '\\' && p[1]) {

                        if (res_len + 2 < sizeof(result)) {
                            result[res_len++] = *p++;
                            result[res_len++] = *p++;
                        } else {
                            break;
                        }

                        continue;
                    }

                    if (res_len + 1 < sizeof(result))
                        result[res_len++] = *p;

                    p++;
                }

                if (*p == '"')
                    p++;
            }

            while (isspace((unsigned char)*p))
                p++;

            if (*p != '\0') {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("invalid string syntax\n",
                    GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

                return (var){ .type = BC_NULL };
            }

            bool dquote = false;
            bool squote = false;

            for (size_t i = 0; operation[i]; i++) {
                unsigned char chr = (unsigned char)operation[i];

                if (chr == '"' && !squote && !is_escaped(operation, i)) {
                    dquote = !dquote;
                    continue;
                }

                if (chr == '\'' && !dquote && !is_escaped(operation, i)) {
                    squote = !squote;
                    continue;
                }

                if (!dquote && !squote) {

                    if (!isalnum(chr) &&
                        !strchr("+-/*^%&|=() \t", chr)) {

                        printc("ceval", BC_PROMPT_COLOR, WHITE);
                        printc(": ",
                            WHITE,
                            GET_BASE_COLOR(BC_PROMPT_COLOR));

                        printf("illegal character: '%c'\n", chr);

                        setColor(WHITE);
                        return (var){ .type = BC_NULL };
                    }
                }
            }

            char *final = alloc(res_len + 3);
            if (!final) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("memory allocation error\n",
                    GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

                return (var){ .type = BC_NULL };
            }

            *final = '"';
            memcpy(final + 1, result, res_len);
            final[res_len + 1] = '"';
            final[res_len + 2] = '\0';

            return (var){
                .type = BC_STR,
                .data.s = final
            };
        }

        var tmp = h_atof(operation, mathlib);

        if (tmp.type == BC_FLOAT && isnan(tmp.data.f))
            return (var){ .type = BC_NULL };

        return tmp;
    }

    if (operation[strlen(operation)-1] == '!') {
        int64_t result = s_fact(operation);

        if (result == I64_NAN)
            return (var){ .type = BC_NULL };

        return (var){ .type = BC_INT, .data.i = result };
    }

    return func_section(operation, funcCount, functions, mathlib);
}

var parse_operation(char *operation, const FuncEntry *functions, size_t funcCount, const ops *operator, bool mathlib) {
    char op[0x4] = {0};

    while (is_wrapped_by_parentheses(operation)) {
        size_t len = strlen(operation);
        if (len <= 2)
            break;

        operation[len - 1] = '\0';
        memmove(operation, operation + 1, len);

        trim(operation);
        trimEnd(operation);
    }

    int16_t op_pos = find_main_operator_full(operation, operator, op);

    if (op_pos == -1)
        return parse_single(operation, functions, funcCount, mathlib);

    char buffer[0x100];
    strncpy(buffer, operation, sizeof(buffer)-1);
    buffer[sizeof(buffer)-1] = '\0';

    buffer[op_pos] = '\0';

    char *num1 = buffer;
    char *num2 = buffer + op_pos + strlen(op);

    trim(num1); trimEnd(num1);
    trim(num2); trimEnd(num2);

    if (!*num1 || !*num2) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("invalid syntax\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return (var){ .type = BC_NULL };
    }

    var val1 = eval(num1, mathlib);

    if (val1.type == BC_NULL)
        return (var){.type = BC_NULL};

    var val2 = eval(num2, mathlib);

    if (val2.type == BC_NULL)
        return (var){.type = BC_NULL};

    var result = calc(val1, op, val2, mathlib);

    return result;
}
