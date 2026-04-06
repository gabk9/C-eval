#include "eval.h"
#include "utils.h"
#include <ctype.h>
#include "s_math.h"

#if !defined(_WIN64) && !defined(__linux__) && !defined(__APPLE__) && !defined(__ANDROID__)
    #error "Operational system not recognized, terminating program!!"
#endif

static uint8_t isnull(int32_t count, ...) {

    if (count < 1) {
        fprintf(stderr, "invalid count for <count>\n");
        exit(EXIT_FAILURE);
    }

    va_list args;
    va_start(args, count);

    uint8_t nullCount = 0;
    for (int8_t i = 0; i < count; i++) {
        void *ptr = va_arg(args, void *);

        if (!ptr)
            nullCount++;
    }

    va_end(args);
    return nullCount;
}

__attribute__((unused))
char *find_top_level_comma(char *s) {
    int16_t level = 0;

    for (char *p = s; *p; p++) {
        if (*p == '(') level++;
        else if (*p == ')') level--;
        else if (*p == ',' && level == 0)
            return p;
    }           
    return NULL;
}

uint16_t count_top_level_commas(const char *s) {
    int32_t level = 0, count = 0;

    for (; *s; s++) {
        if (*s == '(') level++;
        else if (*s == ')') level--;
        else if (*s == ',' && level == 0)
            count++;
    }
    return count;
}

static var numericDebug(const char *buf) {
    if (strncasecmp(buf, BIN_PREF, strlen(BIN_PREF)) == 0) {
        size_t len = strlen(buf);

        const size_t pref_len = strlen(BIN_PREF);

        if (len <= pref_len) {
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("invalid binary literal\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            return (var){.type = BC_FLOAT, .data.f = NAN};
        }

        for (size_t i = pref_len; buf[i]; i++) {
            if (buf[i] != '0' && buf[i] != '1') {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("invalid binary digit: '%c'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, buf[i]);

                break;
            }
        }

        return (var){.type = BC_FLOAT, .data.f = NAN};
    } else if (strncasecmp(buf, HEX_PREF, strlen(HEX_PREF)) == 0) {
        size_t len = strlen(buf);

        const size_t pref_len = strlen(HEX_PREF);

        if (len <= pref_len) {
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("invalid hexadecimal literal\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            return (var){.type = BC_FLOAT, .data.f = NAN};
        }

        for (size_t i = pref_len; buf[i]; i++) {
            if (!isxdigit(buf[i])) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("invalid hexadecimal digit: '%c'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, buf[i]);

                break;
            }
        }

        return (var){.type = BC_FLOAT, .data.f = NAN};
    } else if (strncasecmp(buf, OCT_PREF, strlen(OCT_PREF)) == 0) {
        size_t len = strlen(buf);

        const size_t pref_len = strlen(OCT_PREF);

        if (len <= pref_len) {
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("invalid octal literal\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            return (var){.type = BC_FLOAT, .data.f = NAN};
        }

        for (size_t i = pref_len; buf[i]; i++) {
            if (buf[i] < '0' || buf[i] > '7') {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("invalid octal digit: '%c'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, buf[i]);

                break;
            }
        }

        return (var){.type = BC_FLOAT, .data.f = NAN};
    }

    printc("ceval", BC_PROMPT_COLOR, WHITE);
    printf(": ");
    printc("invalid literal prefix: '%c'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, *buf);

    return (var){.type = BC_FLOAT, .data.f = NAN};
}

static var mathlibPart(char *buf, bool mathlib) {
    if (strcmp(buf, PI_VAR) == 0) return (var){.type = BC_FLOAT, .data.f = PI};
    else if (strcmp(buf, E_VAR) == 0) return (var){.type = BC_FLOAT, .data.f = E};

    uint16_t i = 0;
    while (buf[i] && (isdigit(buf[i]) || buf[i] == '.' || buf[i] == ',' || buf[i] == '-'))
        i++;

    if (i > 0 && strcmp(buf + i, E_VAR) == 0) {
        char temp[0x40];
        strncpy(temp, buf, i);
        temp[i] = '\0';

        var tmp = eval(buf, mathlib);

        if (tmp.type == BC_STR)
            SAFE_FREE(tmp.data.s);

        if (tmp.type == BC_NONE)
            return (var){.type = BC_FLOAT, .data.f = NAN};

        if (tmp.type == BC_FLOAT)
            return (var){.type = BC_FLOAT, .data.f = tmp.data.f * PI};

        return (var){.type = BC_FLOAT, .data.f = (float64)tmp.data.i * PI};
    } else if (i > 0 && strcmp(buf + i, E_VAR) == 0) {
        char temp[0x40];
        strncpy(temp, buf, i);
        temp[i] = '\0';

        var tmp = eval(buf, mathlib);

        if (tmp.type == BC_STR)
            SAFE_FREE(tmp.data.s);

        if (tmp.type == BC_NONE)
            return (var){.type = BC_FLOAT, .data.f = NAN};

        if (tmp.type == BC_FLOAT)
            return (var){.type = BC_FLOAT, .data.f = tmp.data.f * E};

        return (var){.type = BC_FLOAT, .data.f = (float64)tmp.data.i * E};
    }

    return (var){.type = BC_NONE};
}

var h_atof(const char *str, bool mathlib) {

    if (!str || !*str) 
        return (var){.type = BC_FLOAT, .data.f = NAN};

    char buf[0x80];
    strncpy(buf, str, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    trim(buf);
    trimEnd(buf);

    size_t len = strlen(buf);
    if (len == 0)
        return (var){.type = BC_FLOAT, .data.f = NAN};

    if (len > 1 && buf[len-1] == '!') {
        int64_t result = s_fact(buf);

        if (result == I64_NAN)
            return (var){.type = BC_FLOAT, .data.f = NAN};

        return (var){.type = BC_INT, .data.i = result};
    }

    bool isUnaryNeg = false;
    bool isUnaryNot = false;

    while (*buf == '-' || *buf == '~') {
        switch (*buf) {
            case '-':
                isUnaryNeg = !isUnaryNeg;
                break;
            case '~':
                isUnaryNot = !isUnaryNot;
                break;
        }

        memmove(buf, buf+1, strlen(buf)+1);
        trim(buf);
    }


    if (strcmp(buf, NONE_VAR) == 0) {
        if (isUnaryNeg) {
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("bad operand type for unary negative(-): '"NONE_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
        } else if (isUnaryNot) {
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("bad operand type for unary not(~): '"NONE_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
        }

        return (var){.type = BC_FLOAT, .data.f = NAN};
    }

    bool isInf = strcasecmp(buf, INF_VAR) == 0;
    if (mathlib && isInf) {

        if (isInf && strcmp(buf, INF_VAR) != 0)
            return (var){.type = BC_FLOAT, .data.f = 0.0};

        if (isUnaryNot) {
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("bad operand type for unary not(~) '"INF_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
            return (var){.type = BC_FLOAT, .data.f = NAN};
        }

        return (var){.type = BC_FLOAT, .data.f = isUnaryNeg ? -INFINITY : INFINITY};
    }

    bool isAns = mathlib && strcmp(buf, ANS_VAR) == 0;

    if (isAns) {
        if (Ans.type == BC_NONE) {
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("'ans' is undefined\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            return (var){.type = BC_FLOAT, .data.f = NAN};
        } else if (Ans.type == BC_STR) {
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("cannot operate with strings\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            return (var){.type = BC_FLOAT, .data.f = NAN};
        }

        return Ans;

    } 

    if (isUnaryNeg) {
        if (!*buf) {
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("missing value for unary negative(-)\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            return (var){.type = BC_FLOAT, .data.f = NAN};
        }

        float64 num;
        int64_t num1;
        bool isInt = false;

        if (isAns && Ans.type != BC_STR)
            return Ans;

        var tmp = eval(buf, mathlib);

        if (tmp.type == BC_NONE)
            return (var){.type = BC_FLOAT, .data.f = NAN};

        switch (tmp.type) {
            case BC_BOOL:
                num1 = (int64_t)tmp.data.b;
                isInt = true;
                break;
            case BC_CHR:
            case BC_INT:
                num1 = tmp.data.i;
                isInt = true;
                break;
            case BC_FLOAT:
                num = tmp.data.f;
                break;
            default:
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("unary not(~) requires an argument of type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

                if (tmp.type == BC_STR)
                    SAFE_FREE(tmp.data.s);

                return (var){.type = BC_FLOAT, .data.f = NAN};
        }

        if (!isInt) {
            if (isnan(num))
                return (var){.type = BC_FLOAT, .data.f = NAN};

            if (num < MIN_SAFE_INT64_D || num > MAX_SAFE_INT64_D) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("numeric overflow (too large)\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

                return (var){.type = BC_FLOAT, .data.f = NAN};
            }

            return (var){.type = BC_FLOAT, .data.f = -num};
        } else {
            if (num1 < INT64_MIN || num1 > INT64_MAX) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("numeric overflow (too large)\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

                return (var){.type = BC_FLOAT, .data.f = NAN};
            }

            return (var){.type = BC_INT, .data.i = -num1};
        }
    } else if (isUnaryNot) {
        if (!*buf) {
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("missing value for unary not(~)\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            return (var){.type = BC_FLOAT, .data.f = NAN};
        }

        int64_t num1;

        if (isAns && Ans.type != BC_STR)
            return Ans;

        var tmp = eval(buf, mathlib);

        if (tmp.type == BC_NONE)
            return (var){.type = BC_FLOAT, .data.f = NAN};

        switch (tmp.type) {
            case BC_BOOL:
                num1 = (int64_t)tmp.data.b;
                break;
            case BC_CHR:
            case BC_INT:
                num1 = tmp.data.i;
                break;
            default:
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("unary not(~) requires an argument of type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

                if (tmp.type == BC_STR)
                    SAFE_FREE(tmp.data.s);

                return (var){.type = BC_FLOAT, .data.f = NAN};
        }

        return (var){.type = BC_INT, .data.i = ~num1};
    }

    if (strcmp(buf, TRUE_VAR) == 0)
        return (var){.type = BC_BOOL, .data.b = true};
    else if (strcmp(buf, FALSE_VAR) == 0)
        return (var){.type = BC_BOOL, .data.b = false};
    else if (mathlib && strcmp(buf, RAND_MAX_VAR) == 0)
        return (var){.type = BC_INT, .data.i = RAND_MAX};

    len = strlen(buf);

    if (isBetweenQuotes(buf, SINGLE_QUOTES)) {
        if (!injectEscape(buf, "ceval"))
            return (var){.type = BC_FLOAT, .data.f = NAN};

        size_t oldLen = len;
        len = strlen(buf);
        bool isNullChr = !buf[1] && oldLen != len;
        
        if (buf[len-1] == '\'') {
            buf[len-1] = '\0';
            len--;
        }
        if (*buf == '\'') {
            memmove(buf, buf+1, len+1);
            len--;
        }

        unsigned char chr = (unsigned char)*buf;
        if (len > 1) {
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("to use single quotes it must be a single character\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            return (var){.type = BC_FLOAT, .data.f = NAN};
        } else if (!isNullChr && len < 1) {
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("missing the character inside quotes\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            return (var){.type = BC_FLOAT, .data.f = NAN};
        } else if (chr > 0x80) {
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("cannot work with multi-byte characters\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            return (var){.type = BC_FLOAT, .data.f = NAN};
        }

        return (var){.type = BC_CHR, .data.i = chr};
    }

    bool is_hex = isHex(buf);

    bool is_octal = isOct(buf);
    
    bool is_bin = isBin(buf);

    if (*buf == '0' && buf[1] && buf[1] != '.'&& !is_bin && !is_octal && !is_hex)
        return numericDebug(buf);

    if (mathlib) {
        var tmp = mathlibPart(buf, mathlib);

        if (tmp.type != BC_NONE)
            return tmp;
    }

    if (is_hex)
        return (var){.type = BC_INT, .data.i = hex_to_long(buf)};
    else if (is_octal)
        return (var){.type = BC_INT, .data.i = strtol(buf+strlen(OCT_PREF), NULL, 8)};
    else if (is_bin)
        return (var){.type = BC_INT, .data.i = parseBinToInt(buf)};

    if (*buf == '"' && buf[len-1] == '"') {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("cannot operate with '"STR_VAR"' type values\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return (var){.type = BC_FLOAT, .data.f = NAN};
    }

    if (!isalldigit(buf)) {
        bool shouldError;
        bool isValid = isBcVariable(buf, &shouldError);

        if (!isValid && shouldError) {
            for (size_t i = 0; buf[i]; i++) {
                if (strchr("()\"'!. _+-/~*^%&|<>", buf[i]))
                    continue;

                if (!isalnum((unsigned char)buf[i])) {
                    unsigned char chr = buf[i];
                    printc("ceval", BC_PROMPT_COLOR, WHITE);
                    printc(": ", WHITE, GET_BASE_COLOR(BC_PROMPT_COLOR));

                    if (chr < 0x80)
                        printf("illegal character: '%c'\n", chr);
                    else
                        printf("illegal character: '"HEX_PREF"%X'\n", chr);

                    setColor(WHITE);
                    return (var){ .type = BC_FLOAT, .data.f = NAN };
                }
            }

            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("invalid syntax\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            return (var){.type = BC_FLOAT, .data.f = NAN};
        }
    }

    bool isFloat = strchr(buf, '.');

    if (!isFloat) {
        char *end;
        int64_t val = strtoll(buf, &end, 10);

        if (*end == '\0') {
            return (var){.type = BC_INT, .data.i = val};
        }
    }

    char *end;
    float64 result = strtod(buf, &end);

    if (*end != '\0')
        return (var){.type = BC_INT, .data.i = 0};

    return (var){.type = BC_FLOAT, .data.f = result};
}

int64_t parseBinToInt(const char *str) {
    int64_t n = 0;
    int32_t bits = 0;

    const size_t pref_len = strlen(BIN_PREF);
    for (uint16_t i = pref_len; str[i]; i++) {
        n = (n << 1) | (str[i] - '0');
        bits++;
    }

    if (str[pref_len] == '1') {
        n -= 1 << bits;
    }

    return n;
}

static uint16_t countCommaOutsideQuotesAndParenthesis(const char *str, uint8_t quoteType) {
    uint16_t count = 0;
    bool insideQuotes = false;
    int32_t parenLevel = 0;

    if (!str) {
        return 0;
    }

    while (*str) {
        if (*str == (char)quoteType && parenLevel == 0) {
            insideQuotes = !insideQuotes;
        }
        else if (*str == '(' && !insideQuotes) {
            parenLevel++;
        }
        else if (*str == ')' && !insideQuotes) {
            if (parenLevel > 0)
                parenLevel--;
        }
        else if (*str == ',' && !insideQuotes && parenLevel == 0) {
            count++;
        }

        str++;
    }

    return count;
}

__attribute__((unused))
static uint16_t countCommaOutsideQuotes(const char *str, uint8_t quoteType) {
    uint16_t count = 0;
    bool insideQuotes = false;
    
    if (str == NULL) {
        return 0;
    }
    
    while (*str) {
        if (*str == (char)quoteType) {
            insideQuotes = !insideQuotes;
        } else if (*str == ',' && !insideQuotes) {
            count++;
        }
        str++;
    }
    
    return count;
}

__attribute__((unused))
static uint16_t countCommaOutsideParenthesis(const char *str) {
    uint16_t count = 0;
    int32_t parenLevel = 0;

    if (!str) {
        return 0;
    }

    while (*str) {
        if (*str == '(') {
            parenLevel++; 
        } else if (*str == ')') {
            if (parenLevel > 0)
                parenLevel--;
        } else if (*str == ',' && parenLevel == 0) {
            count++;
        }
        str++;
    }

    return count;
}

void bc_man(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return;
    operation = p;

    for (char *c = operation; *c; c++) {
        if (*c != '(' && *c != ')' && *c != ' ') {
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("man() does not take arguments\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            return;
        }
    }

    print_manual();

    return;
}

char *bc_parse_str(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NULL;
    operation = p;

    size_t len = strlen(operation);

    if (!len || countCommaOutsideQuotesAndParenthesis(operation, '"') != 0) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("str() requires exactly 1 argument\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NULL;
    }

    char *buff = var2str(eval(operation, true));

    if (!buff)
        return NULL;

    len = strlen(buff);
    if (*buff != '"' && buff[len-1] != '"') {
        char *buff2 = malloc(len+3);
        buff2[len+3] = '\0';

        snprintf(buff2, len+3, "\"%s\"", buff);

        SAFE_FREE(buff);
        return buff2;
    }

    return buff;
}

float64 bc_float(char *operation) {

    char *p = strchr(operation, '(');
    if (!p)
        return NAN;

    operation = p;

    if (countCommaOutsideQuotesAndParenthesis(operation, '"') != 0) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc(""FLOAT_VAR"() requires exactly 1 argument\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    var buff = eval(operation, true);

    switch (buff.type) {
        case BC_NONE:
            return NAN;
        case BC_BOOL:
            return (float64)buff.data.b;
        case BC_CHR:
        case BC_INT:
            return (float64)buff.data.i;
        case BC_FLOAT:
            return buff.data.f;
        default:
            break;
    }

    size_t len = strlen(buff.data.s);

    if (len >= 2) {
        memmove(buff.data.s, buff.data.s + 1, len - 2);
        buff.data.s[len - 2] = '\0';
    }

    len = strlen(buff.data.s);

    if (!isalldigit(buff.data.s) || len < 1) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("invalid literal for "FLOAT_VAR"()\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        SAFE_FREE(buff.data.s);
        return NAN;
    }

    var tmp = h_atof(buff.data.s, false);
    SAFE_FREE(buff.data.s);

    switch (tmp.type) {
        case BC_CHR:
        case BC_INT:
            return (float64)tmp.data.i;
        case BC_FLOAT:
            if (isnan(tmp.data.f))
                return NAN;

            return tmp.data.f;
        default:
            return NAN;
    }
}

int64_t bc_int(char *operation) {

    char *p = strchr(operation, '(');
    if (!p)
        return I64_NAN;

    operation = p;

    if (countCommaOutsideQuotesAndParenthesis(operation, '"') != 0) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc(""INT_VAR"() requires exactly 1 argument\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return I64_NAN;
    }

    var buff = eval(operation, true);

    switch (buff.type) {
        case BC_NONE:
            return I64_NAN;
        case BC_BOOL:
            return (int64_t)buff.data.b;
        case BC_CHR:
        case BC_INT:
            return buff.data.i;
        case BC_FLOAT:
            return (int64_t)buff.data.f;
        default:
            break;
    }

    size_t len = strlen(buff.data.s);

    if (len >= 2) {
        memmove(buff.data.s, buff.data.s + 1, len - 2);
        buff.data.s[len - 2] = '\0';
    }

    len = strlen(buff.data.s);

    if (!isalldigit(buff.data.s) || len < 1) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("invalid literal for "INT_VAR"()\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        SAFE_FREE(buff.data.s);
        return I64_NAN;
    }

    var tmp = h_atof(buff.data.s, false);
    SAFE_FREE(buff.data.s);

    switch (tmp.type) {
        case BC_CHR:
        case BC_INT:
            return tmp.data.i;
        case BC_FLOAT:
            if (isnan(tmp.data.f))
                return I64_NAN;

            return (int64_t)tmp.data.f;
        default:
            return I64_NAN;
    }
}

char *bc_typeof(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NULL;
    operation = p;

    var buff = eval(operation, true);

    if (buff.type == BC_STR)
        SAFE_FREE(buff.data.s);
    else if (buff.type == BC_NONE)
        return NULL;

    char tmp[0x20] = {0};
    getItemTypeStr(tmp, sizeof(tmp), buff);

    size_t extra = strlen(tmp) + 3;
    char *result = malloc(extra);

    if (!result)
        return NULL;

    snprintf(result, extra, "\"%s\"", tmp);

    return result;
}

int64_t bc_len(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return I64_NAN;
    operation = p+1;

    size_t len = strlen(operation);
    operation[len-1] = '\0';
    len--;

    if (!len || countCommaOutsideQuotesAndParenthesis(operation, '"') != 0) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("len() requires exactly 1 argument\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return I64_NAN;
    }

    var buff = eval(operation, true);

    if (buff.type == BC_NONE)
        return I64_NAN;

    if (buff.type != BC_STR) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("len() requires an argument of type '"STR_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return I64_NAN;
    }

    if (!injectEscape(buff.data.s, "ceval")) {
        SAFE_FREE(buff.data.s);
        return I64_NAN;
    }

    len = strlen(buff.data.s);

    if (buff.data.s[len-1] == '"') {
        buff.data.s[len-1] = '\0';
        len--;
    } if (*buff.data.s == '"') {
        memmove(buff.data.s, buff.data.s+1, len+1);
        len--;
    }

    SAFE_FREE(buff.data.s);

    return (int64_t)len;
}

float64 s_abs(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    if (tmp.type == BC_NONE)
        return NAN;

    if (tmp.type == BC_STR)
        SAFE_FREE(tmp.data.s);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("abs() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return fabs(num);
}

float64 s_miles(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("mi() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return KM_TO_MI(num);
}

float64 s_km(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("km() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return MI_TO_KM(num);
}

float64 s_pounds(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("lb() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return KG_TO_LB(num);
}

float64 s_kg(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("kg() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return LB_TO_KG(num);
}

float64 s_feet(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("feet() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return M_TO_FT(num);
}

float64 s_meter(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("meter() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return FT_TO_M(num);
}

float64 s_fah(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("fah() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return C_TO_F(num);
}

float64 s_cel(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("cel() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return F_TO_C(num);
}

char *s_oct(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NULL;
    operation = p;

    var tmp = eval(operation, true);

    int64_t val1 = 0;

    switch (tmp.type) {
        case BC_BOOL:
            val1 = (int64_t)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            val1 = tmp.data.i;
            break;
        case BC_FLOAT:
        case BC_STR:
            if (tmp.type == BC_FLOAT && isnan(tmp.data.f))
                return NULL;

            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("oct() requires an argument of type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            if (tmp.type == BC_STR)
                SAFE_FREE(tmp.data.s);

            return NULL;
        default:
            return NULL;
    }


    bool isNeg = val1 < 0;

    if (isNeg)
        val1 = -val1;

    const size_t size = 0x80;

    char *buffer = malloc(size);
    if (!buffer)
        return NULL;

    snprintf(buffer, size, isNeg ? "\"-"OCT_PREF"%"PRIo64"\"" : "\""OCT_PREF"%"PRIo64"\"", val1);

    return buffer;
}

char *s_lower(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NULL;
    operation = p;

    var buff = eval(operation, true);

    if (buff.type == BC_NONE)
        return NULL;

    if (buff.type != BC_STR) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("lower() requires an argument of type '"STR_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NULL;
    }

    for (size_t i = 0; buff.data.s[i]; i++) {
        char chr = buff.data.s[i];

        if (isupper(chr)) {
            buff.data.s[i] = tolower(chr);
            continue;
        }

        buff.data.s[i] = chr;
    }

    return buff.data.s;
}

char *s_upper(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NULL;
    operation = p;

    var buff = eval(operation, true);

    if (buff.type == BC_NONE)
        return NULL;

    if (buff.type != BC_STR) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("upper() requires an argument of type '"STR_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NULL;
    }

    for (size_t i = 0; buff.data.s[i]; i++) {
        char chr = buff.data.s[i];

        int32_t backslashes = 0;
        size_t j = i;

        while (j > 0 && buff.data.s[j-1] == '\\') {
            backslashes++;
            j--;
        }

        bool escaped = backslashes & 1;

        if (escaped && strchr("ntbra'\"?fv0\\", chr)) {
            buff.data.s[i] = chr;
            continue;
        }

        if (islower(chr))
            buff.data.s[i] = toupper(chr);
    }

    return buff.data.s;
}

char *s_chr(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NULL;
    operation = p;

    var tmp = eval(operation, true);

    int64_t num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
        case BC_STR:
            if (tmp.type == BC_FLOAT && isnan(tmp.data.f))
                return NULL;

            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("chr() requires an argument of type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            if (tmp.type == BC_STR)
                SAFE_FREE(tmp.data.s);

            return NULL;
        default:
            return NULL;
    }

    if (tmp.type == BC_FLOAT && isnan(tmp.data.f))
        return NULL;

    int64_t value = (int64_t)num;

    if (value < 0 || value > 127) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("chr() requires an argument of type '"INT_VAR"' between 0 <= x <= 127\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NULL;
    }

    char *chr = malloc(5);
    if (!chr)
        return NULL;

    switch (value) {
        case 0:  strcpy(chr + 1, "\\0"); break;
        case 7:  strcpy(chr + 1, "\\a"); break;
        case 8:  strcpy(chr + 1, "\\b"); break;
        case 9:  strcpy(chr + 1, "\\t"); break;
        case 10: strcpy(chr + 1, "\\n"); break;
        case 11: strcpy(chr + 1, "\\v"); break;
        case 12: strcpy(chr + 1, "\\f"); break;
        case 13: strcpy(chr + 1, "\\r"); break;
        case 34: strcpy(chr + 1, "\\\""); break;
        case 39: strcpy(chr + 1, "\\'"); break;
        case 63: strcpy(chr + 1, "\\?"); break;
        case 92: strcpy(chr + 1, "\\\\"); break;
        default:
            if (value < 32 || value == 127) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("chr() does not work with certain control and escape characters\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

                SAFE_FREE(chr);
                return NULL;
            }
            chr[1] = (char)value;
            chr[2] = '\0';
            break;
    }

    int32_t len = (chr[2] == '\0') ? 2 : 3;

    *chr = '"';
    chr[len] = '"';
    chr[len + 1] = '\0';

    return chr;
}

char *s_hex(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NULL;
    operation = p;

    var tmp = eval(operation, true);

    int64_t val1 = 0;

    switch (tmp.type) {
        case BC_BOOL:
            val1 = (int64_t)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            val1 = tmp.data.i;
            break;
        case BC_FLOAT:
        case BC_STR:
            if (tmp.type == BC_FLOAT && isnan(tmp.data.f))
                return NULL;

            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("hex() requires an argument of type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            if (tmp.type == BC_STR)
                SAFE_FREE(tmp.data.s);

            return NULL;
        default:
            return NULL;
    }

    const size_t size = 0x80;
    char *buffer = malloc(size);
    if (!buffer)
        return NULL;

    int64_to_hex_min(val1, buffer, size);

    for (uint16_t i = strlen(HEX_PREF) + 1; buffer[i]; i++)
        buffer[i] = toupper((unsigned char)buffer[i]);

    return buffer;
}

char *s_bin(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NULL;
    operation = p;

    var tmp = eval(operation, true);

    int64_t val1 = 0;

    switch (tmp.type) {
        case BC_BOOL:
            val1 = (int64_t)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            val1 = tmp.data.i;
            break;
        case BC_FLOAT:
        case BC_STR:
            if (tmp.type == BC_FLOAT && isnan(tmp.data.f))
                return NULL;

            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("bin() requires an argument of type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            if (tmp.type == BC_STR)
                SAFE_FREE(tmp.data.s);

            return NULL;
        default:
            return NULL;
    }

    int64_t n = val1;

    uint64_t u = (uint64_t)n;

    char buf[0x41];
    buf[64] = '\0';

    for (int32_t i = 63; i >= 0; i--) {
        buf[i] = (u & 1) ? '1' : '0';
        u >>= 1;
    }

    int16_t start = 0;
    while (start < 63 && buf[start] == buf[0] && buf[start + 1] == buf[0])
        start++;

    int16_t len = 64 - start;

    char *result = malloc(len + 5); 

    if (!result)
        return NULL;

    char *c = result;

    const size_t pref_len = strlen(BIN_PREF);

    c[0] = '"';
    memcpy(c + 1, BIN_PREF, pref_len);

    memcpy(c + 1 + pref_len, buf + start, len);

    c[1+pref_len+len] = '"';
    c[2+pref_len+len] = '\0';

    return result;
}

int64_t s_trunc(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return I64_NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("trunc() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return I64_NAN;
        default:
            return I64_NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return I64_NAN;

    return (int64_t)trunc(num);
}

float64 s_rad(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("deg2rad() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return DEG_TO_RAD(num);
}

float64 s_gon(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("rad2gon() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;  

    return RAD_TO_GON(num);
}

float64 s_deg(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("rad2deg() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return RAD_TO_DEG(num);
}

float64 s_sqrt(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("sqrt() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    if (num < 0) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("sqrt() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"' and non negative\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    return sqrt(num);
}

int64_t s_scale(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return I64_NAN;
    operation = p;

    var tmp = eval(operation, true);

    switch (tmp.type) {
        case BC_NONE:
            return I64_NAN;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("scale() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return I64_NAN;
        case BC_FLOAT:
            break;
        default:
            return 0;
    }

    char buff1[0x40] = {0};
    num_snprintf(buff1, sizeof(buff1), tmp);

    char *buff2 = strchr(buff1, '.');

    if (!buff2)
        return 0;

    return strlen(buff2 + 1);
}

float64 s_sin(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("sin() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    float64 result = sin(num);

    if (fabs(result) < 1e-6)
        result = 0.0;

    return result;
}

float64 s_asin(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("asin() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    if (num < -1.0 || num > 1.0) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("asin() is defined only for -1 <= x <= 1\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    return asin(num);
}

float64 s_cot(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("cot() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    float64 t = tan(num);

    if (fabs(t) < 1e-12) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("cot() is undefined for %.10g rad\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, num);

        return NAN;
    }

    return 1.0 / t;
}

float64 s_acot(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("acot() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return PI / 2.0 - atan(num);
}

float64 s_cos(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = h_atof(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("cos() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    float64 result = cos(num);

    if (fabs(result) < 1e-6)
        result = 0.0;

    return result;
}

float64 s_acos(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("acos() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    if (num < -1.0 || num > 1.0) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("acos() is defined only for -1 <= x <= 1\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    return acos(num);
}

float64 s_tan(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 angle = 0;

    switch (tmp.type) {
        case BC_BOOL:
            angle = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            angle = tmp.data.i;
            break;
        case BC_FLOAT:
            angle = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("tan() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(angle))
        return NAN;

    float64 modPi = fmod(fabs(angle), PI);
    if (fabs(modPi - PI / 2.0) < 1e-8) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("tan() is undefined for %.10g rad\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, angle);

        return NAN;
    }

    float64 result = tan(angle);

    if (fabs(result) < 1e-6)
        result = 0.0;

    return result;
}

float64 s_atan(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = h_atof(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("atan() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return atan(num);
}

float64 s_ln(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("ln() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return log(num);
}

float64 s_log10(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("log10() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return log10(num);
}

float64 s_log2(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("log2() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return log2(num);
}

float64 s_tet(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p+1;
    operation[strlen(operation)-1] = '\0';

    char *comma = find_top_level_comma(operation);
    
    if (!comma) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("tet() requires exactly 2 arguments\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    *comma = '\0';
    char *baseStr = operation;
    char *tetStr = comma + 1;
    
    uint8_t nullCount = isnull(2, baseStr, tetStr);
    if (nullCount) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("tet() requires exactly 2 arguments (missing %"PRIu8")\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, nullCount);

        return NAN;
    }

    trim(baseStr);
    trim(tetStr);

    var debug1 = eval(baseStr, true);

    float64 base = 0;

    switch (debug1.type) {
        case BC_BOOL:
            base = (float64)debug1.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            base = debug1.data.i;
            break;
        case BC_FLOAT:
            base = debug1.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 1st argument of tet() must be of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(debug1.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (debug1.type == BC_FLOAT && isnan(debug1.data.f))
        return NAN;

    var debug2 = eval(tetStr, true);

    int64_t tet = 0;

    switch (debug2.type) {
        case BC_BOOL:
            tet = (int64_t)debug2.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            tet = debug2.data.i;
            break;
        case BC_FLOAT:
        case BC_STR:
            if (debug2.type == BC_FLOAT && isnan(debug2.data.f))
                return NAN;

            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 2nd argument of tet() must be of type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            if (debug2.type == BC_STR)
                SAFE_FREE(debug2.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tet < 0) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("tet() requires a non negative height\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    if (base == 0 && tet == 0) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("tet(0, 0) is undefined\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);            

        return NAN;
    }

    float64 result = tetration(base, tet);

    if (isnan(result)) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("invalid input for tet()\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
    }

    return result;
}

float64 s_pow(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p+1;
    operation[strlen(operation)-1] = '\0';

    char *comma = find_top_level_comma(operation);
    
    if (!comma) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("pow() requires exactly 2 arguments\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    *comma = '\0';
    char *baseStr = operation;
    char *powerStr = comma + 1;
    
    uint8_t nullCount = isnull(2, baseStr, powerStr);
    if (nullCount) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("pow() requires exactly 2 arguments (missing %"PRIu8")\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, nullCount);

        return NAN;
    }

    trim(baseStr);
    trim(powerStr);

    var debug1 = eval(baseStr, true);

    float64 base = 0;

    switch (debug1.type) {
        case BC_BOOL:
            base = (float64)debug1.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            base = debug1.data.i;
            break;
        case BC_FLOAT:
            base = debug1.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 1st argument of pow() must be of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(debug1.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (debug1.type == BC_FLOAT && isnan(debug1.data.f))
        return NAN;

    var debug2 = eval(powerStr, true);

    float64 power = 0;

    switch (debug2.type) {
        case BC_BOOL:
            power = (float64)debug2.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            power = debug2.data.i;
            break;
        case BC_FLOAT:
            power = debug2.data.f;

            if (isnan(power))
                return NAN;

            if (trunc(power) != power && T_CMP(base, (int64_t)base)) {
                printc("ceval", BC_PROMPT_COLOR, WHITE);
                printf(": ");
                printc("to use negative bases the power must not be of type '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

                return NAN;
            }

            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 2nd argument of pow() must be of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(debug2.data.s);

            return NAN;
        default:
            return NAN;
    }

    float64 result = pow(base, power);

    if (isnan(result)) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("invalid input for pow()\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
    }

    return result;
}

float64 s_root(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p+1;
    operation[strlen(operation)-1] = '\0';

    char *comma = find_top_level_comma(operation);
    
    if (!comma) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("root() requires exactly 2 arguments\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    *comma = '\0';
    char *indexStr = operation;
    char *rootingStr = comma + 1;
    
    uint8_t nullCount = isnull(2, indexStr, rootingStr);
    if (nullCount) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("root() requires exactly 2 arguments (missing %"PRIu8")\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, nullCount);

        return NAN;
    }

    trim(indexStr);
    trim(rootingStr);

    var debug1 = eval(indexStr, true);

    float64 index = 0;

    switch (debug1.type) {
        case BC_BOOL:
            index = (float64)debug1.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            index = debug1.data.i;
            break;
        case BC_FLOAT:
            index = debug1.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 1st argument of root() must be of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(debug1.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (debug1.type == BC_FLOAT && isnan(index))
        return NAN;

    var debug2 = eval(rootingStr, true);

    float64 rooting = 0;

    switch (debug2.type) {
        case BC_BOOL:
            rooting = (float64)debug2.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            rooting = debug2.data.i;
            break;
        case BC_FLOAT:
            rooting = debug2.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 2nd argument of root() must be of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(debug2.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (debug2.type == BC_FLOAT && isnan(rooting))
        return NAN;

    bool invert = false;

    if (index == 0) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("root() requires an index that is not 0\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    if (!T_CMP(index, (int64_t)index)) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("root() requires an index of type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    if (index < 0) {
        invert = true;
        index = -index;
    }

    if (rooting < 0 && (((int64_t)index & 1) == 0)) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("root() requires an odd index when there is a negative number\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    float64 result;

    if (rooting < 0)
        result = -pow(-rooting, 1.0 / index);
    else
        result = pow(rooting, 1.0 / index);

    if (invert)
        result = 1.0 / result;

    return result;
}

float64 s_bmi(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p+1;
    operation[strlen(operation)-1] = '\0';

    char *comma = find_top_level_comma(operation);

    if (!comma) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("bmi() requires exactly 2 arguments\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    *comma = '\0';
    char *weightStr = operation;
    char *heightStr = comma + 1;

    uint8_t nullCount = isnull(2, weightStr, heightStr);
    if (nullCount) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("bmi() requires exactly 2 arguments (missing %"PRIu8")\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, nullCount);

        return NAN;
    }

    trim(weightStr);
    trim(heightStr);

    var debug1 = eval(weightStr, true);

    float64 weight = 0;

    switch (debug1.type) {
        case BC_BOOL:
            weight = (float64)debug1.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            weight = debug1.data.i;
            break;
        case BC_FLOAT:
            weight = debug1.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 1st argument of bmi() must be of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(debug1.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (debug1.type == BC_FLOAT && isnan(weight))
        return NAN;

    var debug2 = h_atof(heightStr, true);

    float64 height = 0;

    switch (debug2.type) {
        case BC_BOOL:
            height = (float64)debug2.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            height = debug2.data.i;
            break;
        case BC_FLOAT:
            height = debug2.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 2nd argument of bmi() must be of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(debug2.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (debug2.type == BC_FLOAT && isnan(height))
        return NAN;

    return BMI(weight, height);
}

float64 s_log(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p+1;
    operation[strlen(operation)-1] = '\0';

    char *comma = find_top_level_comma(operation);

    if (!comma) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("log() requires exactly 2 arguments\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    *comma = '\0';
    char *baseStr = operation;
    char *numStr = comma + 1;

    uint8_t nullCount = isnull(2, baseStr, numStr);
    if (nullCount) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("log() requires exactly 2 arguments (missing %"PRIu8")\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, nullCount);

        return NAN;
    }

    trim(baseStr);
    trim(numStr);

    var debug1 = eval(baseStr, true);

    float64 base = 0;

    switch (debug1.type) {
        case BC_BOOL:
            base = (float64)debug1.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            base = debug1.data.i;
            break;
        case BC_FLOAT:
            base = debug1.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 1st argument of log() must be of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(debug1.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (debug1.type == BC_FLOAT && isnan(base))
        return NAN;

    var debug2 = h_atof(numStr, true);

    float64 num = 0;

    switch (debug2.type) {
        case BC_BOOL:
            num = (float64)debug2.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = debug2.data.i;
            break;
        case BC_FLOAT:
            num = debug2.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 2nd argument of log() must be of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(debug2.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (debug2.type == BC_FLOAT && isnan(num))
        return NAN;

    if (base <= 1 || num <= 0) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("invalid values for log()\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    return log(num) / log(base);
}

float64 s_randFloat(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p+1;
    operation[strlen(operation)-1] = '\0';

    char *comma = find_top_level_comma(operation);

    if (!comma) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("randf() requires exactly 2 arguments\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    *comma = '\0';
    char *str_min = operation;
    char *str_max = comma + 1;

    uint8_t nullCount = isnull(2, str_min, str_max);
    if (nullCount) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("randf() requires exactly 2 arguments (missing %"PRIu8")\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, nullCount);

        return NAN;
    }

    trim(str_max);
    trimEnd(str_max);
    trim(str_min);
    trimEnd(str_min);

    float64 maxLf = 0.0;
    float64 minLf = 0.0;

    var tmp1 = eval(str_max, true);

    maxLf = 0;

    switch (tmp1.type) {
        case BC_BOOL:
            maxLf = (float64)tmp1.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            maxLf = tmp1.data.i;
            break;
        case BC_FLOAT:
            maxLf = tmp1.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 1st argument of randf() must be of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp1.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp1.type == BC_FLOAT && isnan(tmp1.data.f))
        return NAN;

    var tmp = h_atof(str_min, true);

    minLf = 0;

    switch (tmp.type) {
        case BC_BOOL:
            minLf = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            minLf = tmp.data.i;
            break;
        case BC_FLOAT:
            minLf = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 2nd argument of randf() must be of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(tmp.data.f))
        return NAN;

    return random_range_float(minLf, maxLf);
}

int64_t s_randInt(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return I64_NAN;
    operation = p+1;
    operation[strlen(operation)-1] = '\0';

    char *comma = find_top_level_comma(operation);

    if (!comma) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("rand() requires exactly 2 arguments\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return I64_NAN;
    }
    
    *comma = '\0';
    char *str_min = operation;
    char *str_max = comma + 1;
    
    uint8_t nullCount = isnull(2, str_min, str_max);
    if (nullCount) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("rand() requires exactly 2 arguments (missing %"PRIu8")\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, nullCount);

        return I64_NAN;
    }

    trim(str_max);
    trimEnd(str_max);
    trim(str_min);
    trimEnd(str_min);

    int64_t maxInt = 0.0;
    int64_t minInt = 0.0;

    var tmp1 = eval(str_max, true);

    maxInt = 0;

    switch (tmp1.type) {
        case BC_BOOL:
            maxInt = (int64_t)tmp1.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            maxInt = tmp1.data.i;
            break;
        case BC_FLOAT:
        case BC_STR:
            if (tmp1.type == BC_FLOAT && isnan(tmp1.data.f))
                return I64_NAN;

            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 1st argument of rand() must be of type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            if (tmp1.type == BC_STR)
                SAFE_FREE(tmp1.data.s);

            return I64_NAN;
        default:
            return I64_NAN;
    }

    var tmp = eval(str_min, true);

    minInt = 0;

    switch (tmp.type) {
        case BC_BOOL:
            minInt = (int64_t)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            minInt = tmp.data.i;
            break;
        case BC_FLOAT:
        case BC_STR:
            if (tmp.type == BC_FLOAT && isnan(tmp.data.f))
                return I64_NAN;

            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 2nd argument of rand() must be of type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            if (tmp.type == BC_STR)
                SAFE_FREE(tmp.data.s);

            return I64_NAN;
        default:
            return I64_NAN;
    }

    return (int64_t)random_range_int((int32_t)minInt, (int32_t)maxInt);
}

int64_t s_floor(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return I64_NAN;
    operation = p;

    var tmp = h_atof(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("floor() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return I64_NAN;
        default:
            return I64_NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return I64_NAN;

    return (int64_t)floor(num);
}

int64_t s_ceil(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return I64_NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("ceil() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return I64_NAN;
        default:
            return I64_NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return I64_NAN;

    return (int64_t)ceil(num);
}

int64_t s_round(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return I64_NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("round() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return I64_NAN;
        default:
            return I64_NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return I64_NAN;

    return (int64_t)round(num);
}

float64 tetration(float64 base, int32_t height) {
    if (height < 0) return NAN;
    if (height == 0) return 1.0;

    float64 result = base;

    for (int32_t i = 2; i <= height; i++) {
        if (result > log(DBL_MAX) / log(fabs(base)))
            return INFINITY;

        result = pow(base, result);
    }

    return result;
}

bool isprime(int64_t n) {
    if (n < 2) return false;
    
    for (uint64_t i = 2; i * i <= (uint64_t)n; i++)
        if (n % i == 0) return false;
    
    return true;
}

int64_t s_isprime(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return I64_NAN;
    operation = p;

    var tmp = eval(operation, true);

    int64_t num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (int64_t)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
        case BC_STR:
            if (tmp.type == BC_FLOAT && isnan(tmp.data.f))
                return I64_NAN;

            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("isprime() requires an argument of type '"INT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            if (tmp.type == BC_STR)
                SAFE_FREE(tmp.data.s);

            return I64_NAN;
        default:
            return I64_NAN;
    }

    if (num <= 1) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("isprime() requires a number greater than 1\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return I64_NAN;
    }

    return isprime(num);
}

uint64_t fact(int64_t num, int32_t steps) {

    if (num < 0)
        return U64_NAN;

    if (steps <= 0)
        return U64_NAN;

    if (num == 0)
        return 1;

    uint64_t result = 1;

    for (int64_t i = num; i >= 1; i -= steps) {
        if (result > UINT64_MAX / i)
            return U32_NAN;
        result *= i;
    }

    return result;
}

int64_t s_fact(char *operation) {
    char *test = strdup(operation);

    if (!test) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("strdup failed\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return I64_NAN;
    }

    size_t len = strlen(test);
    if (len == 0) {
        SAFE_FREE(test);
        return I64_NAN;
    }

    size_t stepsCount = 0;
    for (int32_t i = len-1; i >= 0; i--) {
        if (test[i] != '!')
            break;

        if (test[i] == '!')
            stepsCount++;
    }

    if (stepsCount == len)
        *test = '\0';
    else if (stepsCount == 0) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("to factor you need '!' as a suffix\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        SAFE_FREE(test);
        return I64_NAN;
    } else if (stepsCount > 0)
        test[len-stepsCount] = '\0';

    trimEnd(test);

    if (strlen(test) < 1) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("missing a value to factor\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        SAFE_FREE(test);
        return I64_NAN;
    }

    var tmp = eval(test, true);

    SAFE_FREE(test);

    if (tmp.type != BC_INT && tmp.type != BC_CHR && tmp.type != BC_BOOL) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("you can only factor '"INT_VAR"' values\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        if (tmp.type == BC_STR)
            SAFE_FREE(tmp.data.s);

        return I64_NAN;
    }

    int64_t num = (tmp.type == BC_BOOL) ? (int64_t)tmp.data.b : tmp.data.i;

    if (num < 0) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("cannot factor negative values\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return I64_NAN;
    }

    uint64_t result =  fact(num, stepsCount);

    if (result >= INT64_MAX || result == U32_NAN) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("numeric overflow (too large)\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return I64_NAN;
    }

    return result;
}

float64 s_sign(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p;

    var tmp = eval(operation, true);

    float64 num = 0;

    switch (tmp.type) {
        case BC_BOOL:
            num = (float64)tmp.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            num = tmp.data.i;
            break;
        case BC_FLOAT:
            num = tmp.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("sign() requires an argument of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(tmp.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (tmp.type == BC_FLOAT && isnan(num))
        return NAN;

    return signbit(num);
}

float64 s_sum(char *operation) {
    char *p = strchr(operation, '(');
    if (!p)
        return NAN;
    operation = p+1;
    operation[strlen(operation)-1] = '\0';

    uint16_t commaCount = count_top_level_commas(operation);

    if (commaCount < 1 || commaCount > 2) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc(
            "sum() function requires at least 2 arguments and at most 3 arguments\n", 
            GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE
        );

        return NAN;
    }

    char *comma1 = find_top_level_comma(operation);
    if (!comma1)
        return NAN;

    char *comma2 = find_top_level_comma(comma1 + 1);

    char *initStr = operation;
    char *endStr;
    char *diffStr = NULL;

    *comma1 = '\0';
    endStr = comma1 + 1;

    if (comma2) {
        *comma2 = '\0';
        diffStr = comma2 + 1;
    }

    uint8_t nullCount;
    if (diffStr)
        nullCount = isnull(3, initStr, endStr, diffStr);
    else
        nullCount = isnull(2, initStr, endStr);

    if (nullCount) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("sum() missing %"PRIu8" argument(s)\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, nullCount);

        return NAN;
    }

    trim(initStr);
    trim(endStr);
    if (diffStr)
        trim(diffStr);

    var debug1 = eval(initStr, true);

    float64 init = 0;

    switch (debug1.type) {
        case BC_BOOL:
            init = (float64)debug1.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            init = debug1.data.i;
            break;
        case BC_FLOAT:
            init = debug1.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 1st argument of sum() must be of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(debug1.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (debug1.type == BC_FLOAT && isnan(debug1.data.f))
        return NAN;

    var debug2 = eval(endStr, true);

    float64 end = 0;

    switch (debug2.type) {
        case BC_BOOL:
            end = (float64)debug2.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            end = debug2.data.i;
            break;
        case BC_FLOAT:
            end = debug2.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 2nd argument of sum() must be of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(debug2.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (debug2.type == BC_FLOAT && isnan(debug2.data.f)) 
        return NAN;

    char *defaultDiff = "1";
    char *tmp3 = diffStr ? diffStr : defaultDiff;

    var debug3 = eval(tmp3, true);

    float64 diff = 0;

    switch (debug3.type) {
        case BC_BOOL:
            diff = (float64)debug3.data.b;
            break;
        case BC_CHR:
        case BC_INT:
            diff = debug3.data.i;
            break;
        case BC_FLOAT:
            diff = debug3.data.f;
            break;
        case BC_STR:
            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("the 3rd argument of sum() must be of type '"INT_VAR"' or '"FLOAT_VAR"'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            SAFE_FREE(debug3.data.s);

            return NAN;
        default:
            return NAN;
    }

    if (debug3.type == BC_FLOAT && isnan(debug3.data.f))
        return NAN;

    if (diff <= 0.0) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("step value must be greater than 0\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    if ((diff > 0.0 && init > end) || (diff < 0.0 && init < end)) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("step direction does not progress from X to Y\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    float64 result = gauss_range_double(init, end, diff);

    if (isnan(result) || isinf(result)) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("numeric overflow or invalid result during summation\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return NAN;
    }

    return result;
}
