#include "eval.h"

static int32_t eval_depth = 0;
var Ans = { .type = BC_NONE };

var eval(char *operation, bool mathlib) {

    if (!operation)
        return (var){.type = BC_NONE};

    const FuncEntry math_table[] = {
        {.returnType = BC_NONE,    .name = "man",       .fn.n = bc_man},
        {.returnType = BC_BOOL,    .name = "isprime",   .fn.i = s_isprime},
        {.returnType = BC_INT,     .name = "scale",     .fn.i = s_scale},
        {.returnType = BC_INT,     .name = "floor",     .fn.i = s_floor},
        {.returnType = BC_INT,     .name = "ceil",      .fn.i = s_ceil},
        {.returnType = BC_INT,     .name = "round",     .fn.i = s_round},
        {.returnType = BC_INT,     .name = "trunc",     .fn.i = s_trunc},
        {.returnType = BC_INT,     .name = "rand",      .fn.i = s_randInt},
        {.returnType = BC_INT,     .name = "len",       .fn.i = bc_len},
        {.returnType = BC_INT,     .name = INT_VAR,     .fn.i = bc_int},
        {.returnType = BC_FLOAT,   .name = "sqrt",      .fn.f = s_sqrt},
        {.returnType = BC_FLOAT,   .name = "tet",       .fn.f = s_tet},
        {.returnType = BC_FLOAT,   .name = "pow",       .fn.f = s_pow},
        {.returnType = BC_FLOAT,   .name = "root",      .fn.f = s_root},
        {.returnType = BC_FLOAT,   .name = "sin",       .fn.f = s_sin},
        {.returnType = BC_FLOAT,   .name = "asin",      .fn.f = s_asin},
        {.returnType = BC_FLOAT,   .name = "cos",       .fn.f = s_cos},
        {.returnType = BC_FLOAT,   .name = "acos",      .fn.f = s_acos},
        {.returnType = BC_FLOAT,   .name = "tan",       .fn.f = s_tan},
        {.returnType = BC_FLOAT,   .name = "atan",      .fn.f = s_atan},
        {.returnType = BC_FLOAT,   .name = "cot",       .fn.f = s_cot},
        {.returnType = BC_FLOAT,   .name = "acot",      .fn.f = s_acot},
        {.returnType = BC_FLOAT,   .name = "ln",        .fn.f = s_ln},
        {.returnType = BC_FLOAT,   .name = "log10",     .fn.f = s_log10},
        {.returnType = BC_FLOAT,   .name = "log2",      .fn.f = s_log2},
        {.returnType = BC_FLOAT,   .name = "log",       .fn.f = s_log},
        {.returnType = BC_FLOAT,   .name = "signbit",   .fn.f = s_sign},
        {.returnType = BC_FLOAT,   .name = "sum",       .fn.f = s_sum},
        {.returnType = BC_FLOAT,   .name = "deg2rad",   .fn.f = s_rad},
        {.returnType = BC_FLOAT,   .name = "rad2gon",   .fn.f = s_gon},
        {.returnType = BC_FLOAT,   .name = "rad2deg",   .fn.f = s_deg},
        {.returnType = BC_FLOAT,   .name = "randf",     .fn.f = s_randFloat},
        {.returnType = BC_FLOAT,   .name = "fah",       .fn.f = s_fah},
        {.returnType = BC_FLOAT,   .name = "cel",       .fn.f = s_cel},
        {.returnType = BC_FLOAT,   .name = "mi",        .fn.f = s_miles},
        {.returnType = BC_FLOAT,   .name = "km",        .fn.f = s_km},
        {.returnType = BC_FLOAT,   .name = "lb",        .fn.f = s_pounds},
        {.returnType = BC_FLOAT,   .name = "kg",        .fn.f = s_kg},
        {.returnType = BC_FLOAT,   .name = "bmi",       .fn.f = s_bmi},
        {.returnType = BC_FLOAT,   .name = "feet",      .fn.f = s_feet},
        {.returnType = BC_FLOAT,   .name = "meter",     .fn.f = s_meter},
        {.returnType = BC_FLOAT,   .name = "abs",       .fn.f = s_abs},
        {.returnType = BC_FLOAT,   .name = FLOAT_VAR,   .fn.f = bc_float},
        {.returnType = BC_STR,     .name = STR_VAR,     .fn.s = bc_parse_str},
        {.returnType = BC_STR,     .name = CHR_VAR,     .fn.s = s_chr},
        {.returnType = BC_STR,     .name = "bin",       .fn.s = s_bin},
        {.returnType = BC_STR,     .name = "oct",       .fn.s = s_oct},
        {.returnType = BC_STR,     .name = "hex",       .fn.s = s_hex},
        {.returnType = BC_STR,     .name = "lower",     .fn.s = s_lower},
        {.returnType = BC_STR,     .name = "upper",     .fn.s = s_upper},
        {.returnType = BC_STR,     .name = "typeof",    .fn.s = bc_typeof},
    };

    size_t funcCount = sizeof(math_table) / sizeof(*math_table);

    ops operators[] = {
        {.op = "**",  .precedence = 7,  .right_assoc = 1},
        {.op = "<<",  .precedence = 4,  .right_assoc = 0},
        {.op = ">>",  .precedence = 4,  .right_assoc = 0},
        {.op = "<=",  .precedence = 3,  .right_assoc = 0},
        {.op = ">=",  .precedence = 3,  .right_assoc = 0},
        {.op = "==",  .precedence = 2,  .right_assoc = 0},
        {.op = "!=",  .precedence = 2,  .right_assoc = 0},
        {.op = "&&",  .precedence = -2, .right_assoc = 0},

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
        {.op = "||",  .precedence = -3, .right_assoc = 0},

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

        return (var){.type = BC_NONE};
    }

    char tmp[MAX_CHAR] = {0};
    size_t len = strlen(operation);
    memcpy(tmp, operation, len + 1);

    removeComments(tmp);
    trim(tmp);
    trimEnd(tmp);

    if (!*tmp)
        return (var){.type = BC_NONE};

    if (!getInvalidEscape(tmp, "ceval"))
        return (var){.type = BC_NONE};

    eval_depth++;

    var buff = parse_operation(tmp, math_table, funcCount, operators, mathlib);

    switch (buff.type) {
        case BC_NONE:
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

    eval_depth--;
    return buff;
}

char *var2str(var buff) {
    switch (buff.type) {

        case BC_STR:
            return buff.data.s;

        case BC_BOOL: {
            if (buff.data.b == false)
                return strdup(FALSE_VAR);
            else
                return strdup(TRUE_VAR);
        }

        case BC_CHR:
        case BC_INT:
        case BC_FLOAT: {
            const size_t max = 0x60;
            char *tmp = malloc(max);
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
    out.type = BC_NONE;

    if (left.type == BC_NONE || right.type == BC_NONE) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("invalid data type: '"NONE_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
        return out;
    }

    opcode code = get_opcode(operation);

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
                    out.type = BC_NONE;
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

                if (((ssize_t)strlen(multiplied_str) - 2) * (size_t)multiplier > MAX_CHAR) {
                    printc("ceval", BC_PROMPT_COLOR, WHITE);
                    printf(": ");
                    printc("the resultant string must be less than %d characters long\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, MAX_CHAR);
                    return out;
                }

                out.type = BC_STR;
                char *result = strdup(multiplied_str);
                if (!result) {
                    out.type = BC_NONE;
                    return out;
                }

                for (size_t i = 1; i < (size_t)multiplier; i++) {
                    char *old = result;
                    result = bc_strcat(result, multiplied_str);

                    SAFE_FREE(old);
                    if (!result) { 
                        out.type = BC_NONE;
                        return out;
                    }
                }

                out.data.s = result;
                return out;
            }

            case EQ: {
                out.type = BC_BOOL;

                if (left.type != right.type) {
                    out.data.b = false;
                    return out;
                }

                out.data.b = bc_strcmp(left.data.s, right.data.s) == 0;
                return out;
            }

            case NE: {
                out.type = BC_BOOL;

                if (left.type != right.type) {
                    out.data.b = true;
                    return out;
                }

                out.data.b = bc_strcmp(left.data.s, right.data.s) != 0;
                return out;
            }

            case GR: {
                out.type = BC_BOOL;

                if (left.type != right.type) {
                    out.data.b = false;
                    return out;
                }

                out.data.b = bc_strcmp(left.data.s, right.data.s) > 0;
                return out;
            }

            case GRE: {
                out.type = BC_BOOL;

                if (left.type != right.type) {
                    out.data.b = false;
                    return out;
                }

                out.data.b = bc_strcmp(left.data.s, right.data.s) >= 0;
                return out;
            }

            case LS: {
                out.type = BC_BOOL;

                if (left.type != right.type) {
                    out.data.b = false;
                    return out;
                }

                out.data.b = bc_strcmp(left.data.s, right.data.s) < 0;
                return out;
            }

            case LSE: {
                out.type = BC_BOOL;

                if (left.type != right.type) {
                    out.data.b = false;
                    return out;
                }

                out.data.b = bc_strcmp(left.data.s, right.data.s) <= 0;
                return out;
            }

            default: {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("unsupported operand for 'str' type: '%s'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, operation);

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
            num1 = (float64)left.data.i;
            break;
        case BC_FLOAT:
            num1 = left.data.f;
            break;
        default:
            return out;
    }

    switch (right.type) {
        case BC_CHR:
        case BC_BOOL:
        case BC_INT:
            num2 = (float64)right.data.i;
            break;
        case BC_FLOAT:
            num2 = right.data.f;
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
                    out.type = BC_NONE;
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
                out.type = BC_NONE;
                return out;
            }

            result = fmod(num1, num2);
            break;

        case XOR:
            if ((left.type != BC_INT && left.type != BC_CHR) || (right.type != BC_INT && right.type != BC_CHR)) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("'^' requires type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                out.type = BC_NONE;
                return out;
            }

            result = (int64_t)num1 ^ (int64_t)num2;

            break;

        case AND:
            if ((left.type != BC_INT && left.type != BC_CHR) || (right.type != BC_INT && right.type != BC_CHR)) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("'&' requires type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                out.type = BC_NONE;
                return out;
            }

            result = (int64_t)num1 & (int64_t)num2;
            break;

        case OR:
            if (left.type == BC_FLOAT || right.type == BC_FLOAT) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("'|' requires type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                out.type = BC_NONE;
                return out;
            }

            result = (int64_t)num1 | (int64_t)num2;
            break;

        case LS:
            out.type = BC_BOOL;
            out.data.b = (num1 < num2) && !T_CMP(num1, num2);
            break;

        case LSE:
            out.type = BC_BOOL;
            out.data.b = (num1 < num2) || T_CMP(num1, num2);
            break;

        case GR:
            out.type = BC_BOOL;
            out.data.b = (num1 > num2) && !T_CMP(num1, num2);
            break;

        case GRE:
            out.type = BC_BOOL;
            out.data.b = (num1 > num2) || T_CMP(num1, num2);
            break;

        case NE:
            out.type = BC_BOOL;
            out.data.b = !T_CMP(num1, num2);
            break;

        case EQ:
            out.type = BC_BOOL;

            if (isnan(num1) || isnan(num2))
                out.data.b = false;

            else if (isinf(num1) || isinf(num2))
                out.data.b = (num1 == num2);

            else
                out.data.b = T_CMP(num1, num2);

            break;

        case SHL:
            if (num2 < 0 || num2 >= sizeof(uint64_t) * 8) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("invalid shift amount\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                out.type = BC_NONE;
                return out;
            }

            result = (uint64_t)num1 << (uint64_t)num2;
            break;

        case SHR:
            if (num2 < 0 || num2 >= sizeof(int64_t) * 8) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("invalid shift amount\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                out.type = BC_NONE;
                return out;
            }

            result = (int64_t)num1 >> (int64_t)num2;
            break;

        case LAND:
            out.type = BC_BOOL;
            out.data.b = (num1 != 0 && num2 != 0);
            break;

        case LOR:
            out.type = BC_BOOL;
            out.data.b = (num1 != 0 || num2 != 0);
            break;

        default:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("Unknown operator '%s'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, operation);
            out.type = BC_NONE;
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

        return (var){ .type = BC_NONE };
    }

    if (isBetweenQuotes(expr, DOUBLE_QUOTES)) {
        if (count & 1)
            return (var){ .type = BC_BOOL, .data.b = false };
        else
            return (var){ .type = BC_BOOL, .data.b = true };
    }

    if (mathlib) {
        if (Ans.type == BC_STR && strcmp(expr, ANS_VAR) == 0) {
            if (!Ans.data.s)
                return (var){ .type = BC_BOOL, .data.b = false };

            if (count & 1)
                return (var){ .type = BC_BOOL, .data.b = false };
            else
                return (var){ .type = BC_BOOL, .data.b = true };
        }
    }

    var tmp = eval(expr, mathlib);

    if (tmp.type == BC_NONE)
        return (var){.type = BC_NONE};

    float64 num = (tmp.type == BC_BOOL) ? (float64)tmp.data.b : tmp.data.f;
    if (tmp.type == BC_INT) {
        int64_t val = tmp.data.i;
        bool value = (val != 0.0);

        if (count & 1)
            value = !value;

        return (var){ .type = BC_BOOL, .data.b = value };
    }

    if (isnan(num))
        return (var){ .type = BC_NONE };

    bool value = (num != 0.0);

    if (count & 1)
        value = !value;

    return (var){ .type = BC_BOOL, .data.b = value };
}

static var func_section(char *operation, size_t funcCount, const FuncEntry *functions, bool mathlib) {
    char name[0x100] = {0};

    ssize_t parenthesis_index = strchar(operation, '(');
    if (parenthesis_index == -1)
        return h_atof(operation, mathlib);

    memcpy(name, operation, parenthesis_index);
    name[parenthesis_index] = '\0';

    trimEnd(name);

    if (!*name)
        return (var){ .type = BC_NONE };

    if (*operation == '~' || *operation == '-') {
        var tmp = h_atof(operation, mathlib);

        if (tmp.type == BC_FLOAT && isnan(tmp.data.f))
            return (var){ .type = BC_NONE };

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

        return (var){ .type = BC_NONE };
    }

    if (!isValidBcFuncName(name)) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("invalid function name: '%s()'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, name);

        return (var){ .type = BC_NONE };
    }

    if (mathlib) {
        for (size_t i = 0; i < funcCount; i++) {
            if (strcmp(name, functions[i].name) != 0)
                continue;

            switch (functions[i].returnType) {
                case BC_BOOL:
                case BC_INT: {
                    int64_t num = functions[i].fn.i(operation);

                    if (num == I64_NAN)
                        return (var){ .type = BC_NONE };

                    if (functions[i].returnType == BC_BOOL)
                        return (var){ .type = BC_BOOL, .data.b = (bool)num };

                    return (var){ .type = functions[i].returnType, .data.i = num };
                }

                case BC_FLOAT: {
                    float64 num = functions[i].fn.f(operation);

                    if (isnan(num))
                        return (var){ .type = BC_NONE };

                    return (var){ .type = functions[i].returnType, .data.f = num };
                }

                case BC_STR: {
                    char *result = functions[i].fn.s(operation);

                    if (!result)
                        return (var){ .type = BC_NONE };

                    return (var){ .type = functions[i].returnType, .data.s = result};
                }

                case BC_NONE:
                    functions[i].fn.n(operation);
                    return (var){ .type = functions[i].returnType };

                default: {
                    char type[0x14] = {0};

                    getItemTypeStr(type, sizeof(type), (var){.type = functions[i].returnType});

                    printc("ceval", BC_PROMPT_COLOR, WHITE);
                    printf(": ");
                    printc("invalid function with '%s' unknown type: '%s()'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, type, name);

                    return (var){ .type = BC_NONE} ;
                }
            }
        }
    }

    printc("ceval", BC_PROMPT_COLOR, WHITE);
    printf(": ");
    printc("undefined function: '%s()'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, name);

    return (var){ .type = BC_NONE };
}

static var parse_single(char *operation, const FuncEntry *functions, size_t funcCount, bool mathlib) {
    if (mathlib && Ans.type == BC_STR && strcmp(operation, ANS_VAR) == 0)
        return (var){ .type = BC_STR, .data.s = strdup(Ans.data.s)};

    if (*operation == '!')
        return lNot_section(operation, mathlib);

    char *paren = strchr(operation, '(');

    if (!paren) {

        if (strcmp(operation, TRUE_VAR) == 0)
            return (var){ .type = BC_BOOL, .data.b = true };
        else if (strcmp(operation, FALSE_VAR) == 0)
            return (var){ .type = BC_BOOL, .data.b = false };

        size_t len = strlen(operation);

        if (len > 1 && operation[0] == '"') {

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

                    if (*p == '"') {

                        uint16_t backslashes = 0;
                        const char *q = p - 1;

                        while (q >= operation && *q == '\\') {
                            backslashes++;
                            q--;
                        }

                        if ((backslashes & 1) == 0)
                            break;
                    }

                    result[res_len++] = *p;
                    p++;
                }

                if (*p != '"') {
                    printc("ceval", BC_PROMPT_COLOR, WHITE);
                    printf(": ");
                    printc("unclosed quote\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

                    return (var){ .type = BC_NONE };
                }

                p++;
            }

            if (res_len > 0) {

                char *final = malloc(res_len + 3);
                if (!final) {
                    printc("ceval", BC_PROMPT_COLOR, WHITE);
                    printf(": ");
                    printc("memory allocation error\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                    return (var){ .type = BC_NONE };
                }

                final[0] = '"';
                memcpy(final + 1, result, res_len);
                final[res_len + 1] = '"';
                final[res_len + 2] = '\0';

                return (var){ .type = BC_STR, .data.s = final };
            }
        }

        var tmp = h_atof(operation, mathlib);

        if (tmp.type == BC_FLOAT && isnan(tmp.data.f))
            return (var){ .type = BC_NONE };

        return tmp;
    }

    if (operation[strlen(operation)-1] == '!') {
        int64_t result = s_fact(operation);

        if (result == I64_NAN)
            return (var){ .type = BC_NONE };

        return (var){ .type = BC_INT, .data.i = result };
    }

    return func_section(operation, funcCount, functions, mathlib);
}

var parse_operation(char *operation, const FuncEntry *functions, size_t funcCount, ops *operator, bool mathlib) {
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

        return (var){ .type = BC_NONE };
    }

    var val1 = eval(num1, mathlib);

    if (val1.type == BC_NONE)
        return (var){.type = BC_NONE};

    var val2 = eval(num2, mathlib);

    if (val2.type == BC_NONE)
        return (var){.type = BC_NONE};

    var result = calc(val1, op, val2, mathlib);

    return result;
}
