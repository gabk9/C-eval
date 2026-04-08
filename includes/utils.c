#include "utils.h"
#include <ctype.h>
#include <stdarg.h>

opcode get_opcode(const char *op) {
    if (strcmp(op, "+") == 0)
        return ADD;
    else if (strcmp(op, "-") == 0)
        return SUB;
    else if (strcmp(op, "/") == 0)
        return DIV;
    else if (strcmp(op, "*") == 0)
        return MUL;
    else if (strcmp(op, "^") == 0)
        return XOR;
    else if (strcmp(op, "%") == 0)
        return MOD;
    else if (strcmp(op, "&") == 0)
        return AND;
    else if (strcmp(op, "|") == 0)
        return OR;
    else if (strcmp(op, "<") == 0)
        return LS;
    else if (strcmp(op, ">") == 0)
        return GR;
    else if (strcmp(op, "&&") == 0)
        return LAND;
    else if (strcmp(op, "||") == 0)
        return LOR;
    else if (strcmp(op, "<<") == 0)
        return SHL;
    else if (strcmp(op, ">>") == 0)
        return SHR;
    else if (strcmp(op, "<=") == 0)
        return LSE;
    else if (strcmp(op, "==") == 0)
        return EQ;
    else if (strcmp(op, ">=") == 0)
        return GRE;
    else if (strcmp(op, "!=") == 0)
        return NE;

    return UNKNOWN;
}

__attribute__((unused))
eval_ty eval_typeof(const char *str) {
    if (isBetweenQuotes(str, BOTH_QUOTES))
        return BC_STR;
    else if (isBetweenQuotes(str, SINGLE_QUOTES))
        return BC_CHR;

    if (strcmp(str, FALSE_VAR) == 0 || strcmp(str, TRUE_VAR) == 0)
        return BC_BOOL;

    if (strchr(str, '.'))
        return BC_FLOAT;
    else
        return BC_INT;

    return BC_NONE;
}

void extractParenthesis(char *str) {
    size_t len = strlen(str);

    if (len == 0)
        return;

    size_t start = 0;
    size_t end = len - 1;

    while (start < end && str[start] == '(' && str[end] == ')') {
        start++;
        end--;

        while (start <= end && str[start] == ' ')
            start++;

        while (end >= start && str[end] == ' ')
            end--;
    }

    size_t new_len = end - start + 1;
    memmove(str, str + start, new_len);
    str[new_len] = '\0';
    trim(str);
    trimEnd(str);

    return;
}

void initRandom(void) {
#ifdef _WIN64
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    srand((unsigned)counter.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    srand((unsigned)(ts.tv_nsec ^ ts.tv_sec));
#endif
}

void num_snprintf(char *buff, size_t size, var num) {
    if (num.type !=  BC_INT && num.type != BC_FLOAT && num.type != BC_CHR) {
        fprintf(stderr, "ceval: invalid num.type: '%d'\n", num.type);
        exit(EXIT_FAILURE);
    }

    if (num.type == BC_INT || num.type == BC_CHR)
        snprintf(buff, size, "%" PRId64, num.data.i);
    else {
        snprintf(buff, size, "%.*lf", DBL_PRECISION, num.data.f);

        size_t end = strlen(buff) - 1;

        while (end > 0 && buff[end] == '0') {
            if (buff[end] == '0' && buff[end-1] == '.')
                return;
            buff[end--] = '\0';
        }

        if (buff[end] == '.') {
            buff[end] = '\0';
        }
    }
}

void getItemTypeStr(char *buff, size_t size, var item) {
    switch (item.type) {
        case BC_INT:    snprintf(buff, size, INT_VAR);    break;
        case BC_FLOAT:  snprintf(buff, size, FLOAT_VAR);  break;
        case BC_STR:    snprintf(buff, size, STR_VAR);    break;
        case BC_CHR:    snprintf(buff, size, CHR_VAR);    break;
        case BC_BOOL:   snprintf(buff, size, BOOL_VAR);   break;
        case BC_NONE:   snprintf(buff, size, NONE_VAR);   break;
        default:        snprintf(buff, size, "NULL");     break;
    }
}


int32_t bc_strcmp(char *str1, char *str2) {
    char buff1[0x100], buff2[0x100];

    if (isBetweenQuotes(str1, DOUBLE_QUOTES)) {
        size_t len = strlen(str1) - 2;
        strncpy(buff1, str1+1, len);
        buff1[len] = '\0';
    } else
        strncpy(buff1, str1, sizeof(buff1)-1), buff1[sizeof(buff1)-1] = '\0';

    if (isBetweenQuotes(str2, DOUBLE_QUOTES)) {
        size_t len = strlen(str2) - 2;
        strncpy(buff2, str2+1, len);
        buff2[len] = '\0';
    } else
        strncpy(buff2, str2, sizeof(buff2)-1), buff2[sizeof(buff2)-1] = '\0';

    return strcmp(buff1, buff2);
}

char *bc_strcat(const char *dest, const char *src) {
    size_t len1 = strlen(dest);
    size_t len2 = strlen(src);

    if (len1 < 2 || len2 < 2)
        return NULL;

    size_t total = (len1 - 2) + (len2 - 2) + 3;

    char *cat = malloc(total);
    if (!cat)
        return NULL;

    snprintf(cat, total, "\"%.*s%.*s\"",
        (int32_t)(len1 - 2), dest + 1,
        (int32_t)(len2 - 2), src + 1);

    return cat;
}

int8_t getInvalidEscape(const char *str, const char *error_str) {

    bool inDoubleQuotes = false;
    bool inSingleQuotes = false;

    size_t len = strlen(str);

    for (size_t i = 0; i < len; i++) {

        int32_t backslashes = 0;
        size_t j = i;

        while (j > 0 && str[j-1] == '\\') {
            backslashes++;
            j--;
        }

        bool escaped = (backslashes & 1);

        if (str[i] == '"' && !inSingleQuotes && !escaped)
            inDoubleQuotes = !inDoubleQuotes;

        else if (str[i] == '\'' && !inDoubleQuotes && !escaped)
            inSingleQuotes = !inSingleQuotes;

        if (!inDoubleQuotes && !inSingleQuotes)
            continue;

        if (str[i] == '\\') {

            if (i == len - 1) {
                color4 color1 = (strcasecmp(error_str, "ceval") == 0) ? BC_PROMPT_COLOR : WHITE;
                color4 color2 = (color1 == BC_PROMPT_COLOR) ? GET_BASE_COLOR(color1) : WHITE;

                printc("%s", color1, WHITE, error_str);
                printf(": ");
                printc("missing escape char after slash\n", color2, WHITE);

                return 0;
            }

            char chr = str[i+1];

            if (!strchr("ntbra'\"?fv0\\", chr)) {
                color4 color1 = (strcasecmp(error_str, "ceval") == 0) ? BC_PROMPT_COLOR : WHITE;
                color4 color2 = (color1 == BC_PROMPT_COLOR) ? GET_BASE_COLOR(color1) : WHITE;

                printc("%s", color1, WHITE, error_str);
                printf(": ");
                printc("invalid escape character: '\\%c'\n", color2, WHITE, chr);

                return 0;
            }

            i++;
        }
    }

    return 1;
}

void shiftLeft_at(char *str, size_t pos) {
    size_t len = strlen(str);
    if (pos >= len) return;

    memmove(&str[pos], &str[pos + 1], len - pos);
}

bool is_wrapped_by_parentheses(const char *s) {
    int32_t len = strlen(s);

    if (len < 2)
        return false;

    if (s[0] != '(' || s[len - 1] != ')')
        return false;

    int32_t depth = 0;

    for (int32_t i = 0; i < len - 1; i++) {

        if (s[i] == '(')
            depth++;
        else if (s[i] == ')')
            depth--;

        if (depth == 0 && i < len - 2)
            return false;
    }

    return depth == 1;
}

bool isalldigit(const char *s) {
    if (!s || !*s) return false;

    if (*s == '-' || *s == '~')
        s++;

    bool hex = isHex(s);
    bool oct = isOct(s);
    bool bin = isBin(s);

    if (hex)
        return true;

    if (oct)
        return true;
    
    if (bin)
        return true;


    uint16_t dotCount = 0;
    const char *p = s;

    for (; *p; p++) {
        if (*p == '.') {
            if (++dotCount > 1)
                return false;
        }
        else if (*p == 'e' || *p == 'E') {
            //* i'll keep it that way
            // p++;
            // if (*p == '+' || *p == '-')
            //     p++;
            // if (!isdigit((unsigned char)*p))
            //     return false;

            // while (isdigit((unsigned char)*p))
            //     p++;

            // return *p == '\0';
            return false;
        }
        else if (!isdigit((unsigned char)*p)) {
            break;
        }
    }

    if (!*p)
        return true;

    return false;
}

int16_t find_main_operator_full(const char *s, const ops *operators, char *foundOp) {
    int32_t len = strlen(s);

    int32_t depth = 0;
    bool in_single = false;
    bool in_double = false;

    int32_t best_pos = -1;
    int32_t best_prec = 9999;
    int32_t best_assoc = 0;
    (void)best_assoc;

    for (int32_t i = len - 1; i >= 0; i--) {

        char c = s[i];

        if (c == '"' && !in_single) {
            in_double = !in_double;
            continue;
        }
        else if (c == '\'' && !in_double) {
            in_single = !in_single;
            continue;
        }

        if (in_single || in_double)
            continue;

        if (c == '(') {
            depth++;
            continue;
        }
        else if (c == ')') {
            depth--;
            continue;
        }

        if (depth != 0)
            continue;

        for (int32_t j = 0; operators[j].op; j++) {

            int32_t oplen = strlen(operators[j].op);

            int32_t start = i - oplen + 1;

            if (start < 0)
                continue;

            if (strncmp(&s[start], operators[j].op, oplen) == 0) {

                if (oplen == 1) {
                    if (start + 1 < len) {
                        char next = s[start + 1];

                        if ((s[start] == '<' && next == '<') ||
                            (s[start] == '>' && next == '>') ||
                            (s[start] == '&' && next == '&') ||
                            (s[start] == '|' && next == '|')) {
                            continue;
                        }
                    }
                }

                if (strcmp(operators[j].op, "-") == 0) {
                    int32_t k = i - 1;

                    while (k >= 0 && isspace((unsigned char)s[k]))
                        k--;

                    if (k < 0)
                        continue;

                    bool is_prev_operator = false;

                    if (strchr("+-/*^%&|<>", s[k]) || s[k] == '(' || s[k] == ',') {
                        is_prev_operator = true;
                    } else {
                        for (int32_t m = 0; operators[m].op; m++) {
                            int32_t len2 = strlen(operators[m].op);

                            if (k - len2 + 1 >= 0 &&
                                strncmp(&s[k - len2 + 1], operators[m].op, len2) == 0) {
                                is_prev_operator = true;
                                break;
                            }
                        }
                    }

                    if (is_prev_operator)
                        continue;
                }

                int32_t prec = operators[j].precedence;
                int32_t assoc = operators[j].right_assoc;

                bool should_replace = false;

                if (best_pos == -1)
                    should_replace = true;
                else if (prec < best_prec)
                    should_replace = true;
                else if (prec == best_prec) {
                    if (assoc == 0 && start > best_pos)
                        should_replace = true;
                    else if (assoc == 1 && start < best_pos)
                        should_replace = true;
                }

                if (should_replace) {
                    best_pos = start;
                    best_prec = prec;
                    best_assoc = assoc;
                    strcpy(foundOp, operators[j].op);
                    i -= (oplen - 1);
                }
            }
        }
    }

    return best_pos;
}

int16_t injectEscape(char *str, const char *error_str) {
    bool inQuotes = false;

    for (size_t i = 0; str[i]; i++) {
        if ((str[i] == '"' || str[i] == '\'') && (i == 0 || str[i-1] != '\\')) {
            inQuotes = !inQuotes;
            continue;
        }

        if (!inQuotes)
            continue;

        if (str[i] == '\\' && str[i+1]) {
            char next = str[i+1];
            char replace = 0;

            switch(next) {
                case 'n':  replace = '\n'; break;
                case 't':  replace = '\t'; break;
                case 'b':  replace = '\b'; break;
                case 'r':  replace = '\r'; break;
                case 'a':  replace = '\a'; break;
                case '\'': replace = '\''; break;
                case '"':  replace = '"';  break;
                case '?':  replace = '?';  break;
                case '\\': replace = '\\'; break;
                case 'f':  replace = '\f'; break;
                case 'v':  replace = '\v'; break;
                case '0':  replace = '\0'; break;
                default: 
                    printf("%s: invalid escape character: '\\%c'\n", error_str, next);
                    return 0;
            }

            str[i] = replace;
            shiftLeft_at(str, i+1);
        }
    }
    return 1;
}

bool isValidBcFuncName(const char *str) {
    if (!str || !*str)
        return false;

    if (!isalpha((unsigned char)*str) && *str != '_')
        return false;

    size_t len = strlen(str);
    size_t end = len - 1;

    while (end > 0 && str[end] == ' ') end--;

    for (size_t i = 1; i < end; i++) {
        if (!isalnum((unsigned char)str[i]) && str[i] != '_')
            return false;
    }

    return true;
}

paren_status parenthesis_check(const char *str) {
    int32_t level = 0;
    bool in_double_quotes = false;
    bool in_single_quotes = false;

    if (!str) return PAREN_OK;

    for (const char *s = str; *s; s++) {

        int32_t backslashes = 0;
        for (const char *p = s; p > str && *(p - 1) == '\\'; p--)
            backslashes++;

        bool escaped = (backslashes & 1);

        if (*s == '"' && !in_single_quotes && !escaped) {
            in_double_quotes = !in_double_quotes;
            continue;
        }

        if (*s == '\'' && !in_double_quotes && !escaped) {
            in_single_quotes = !in_single_quotes;
            continue;
        }

        if (in_double_quotes || in_single_quotes)
            continue;

        if (*s == '(')
            level++;
        else if (*s == ')') {
            level--;
            if (level < 0)
                return PAREN_MISSING_OPEN;
        }
    }

    if (in_double_quotes || in_single_quotes)
        return PAREN_UNCLOSED_QUOTE;

    if (level > 0)
        return PAREN_MISSING_CLOSE;

    return PAREN_OK;
}

void trimEnd(char *str) {
    uint16_t len = strlen(str);

    for (int16_t i = len - 1; i >= 0; i--)
        if (str[i] != ' ')  {
            str[i + 1] = '\0';
            break;
        }
}

void trim(char *str) {
    if (!str) return;
    
    uint16_t spaces = 0;
    while (str[spaces] == ' ') {
        spaces++;
    }
    
    if (spaces == 0) return;
    
    uint16_t i = 0;
    while (str[spaces + i] != '\0') {
        str[i] = str[spaces+i];
        i++;
    }
    str[i] = '\0';
}

bool isBcVariable(const char *str, bool *shouldError) {
    *shouldError = true;

    if (isBetweenQuotes(str, BOTH_QUOTES)) {
        *shouldError = false;
        return false;
    }

    if (!isalpha((unsigned char)*str))
        return false;

    for (size_t i = 1; str[i]; i++) {
        if (!isalnum((unsigned char)str[i]) && str[i] != '_')
            return false;
    }

    return true;
}

void removeComments(char *str) {
    if (!str) return;

    bool in_double = false;
    bool in_single = false;

    for (size_t i = 0; str[i]; i++) {

        if (str[i] == '"' && !in_single) {
            in_double = !in_double;
        }
        else if (str[i] == '\'' && !in_double) {
            in_single = !in_single;
        }
        else if (str[i] == '#' && !in_double && !in_single) {
            str[i] = '\0';
            return;
        }
    }

    if (in_double || in_single) {
        char *p = strchr(str, '#');
        if (p) *p = '\0';
    }
}

bool isBetweenQuotes(const char *string, int16_t quoteMode) {

    if (!string)
        return false;

    if (quoteMode < 0 || quoteMode > 2)
        return false;

    size_t len = strlen(string);

    if (len < 2)
        return false;

    char first = *string;
    char last  = string[len-1];

    switch (quoteMode) {
        case 0:
            return first == '\'' && last == '\'';
        case 1:
            return first == '"' && last == '"';
        case 2:
            return (first == '\'' && last == '\'') ||
                    (first == '"'  && last == '"');
    }

    return false;
}

bool isValidBcCommand(char *str, char *command) {

    uint16_t len = strlen(command);

    if (strncmp(str, command, strlen(command)) == 0 && (str[len] == '\0' || str[len] == ' '))
        return true;

    return false;
}

void charRm(char *str, int8_t targ) {
    int8_t i = 0, j = 0;

    while (str[i] != '\0') {
        if (str[i] != (char)targ) {
            str[j++] = str[i];
        }
        i++;
    }

    str[j] = '\0';
}


bool isBin(const char *str) {
    if (!str || !*str) return false;

    const size_t bin_index = strlen(BIN_PREF);

    if (strncasecmp(str, BIN_PREF, bin_index) != 0)
        return false;

    uint16_t i = bin_index;
    for (; str[i]; i++) {
        if (str[i] != '1' && str[i] != '0')
            return false;
    }

    return i > bin_index;
}

bool isHex(const char *str) {
    if (!str || !*str) return false;

    const size_t hex_index = strlen(HEX_PREF);

    if (strncasecmp(str, HEX_PREF, hex_index) != 0)
        return false;

    if (!str[hex_index])
        return false;

    size_t i  = hex_index;
    for (; str[i]; i++) {
        if (!isxdigit((unsigned char)str[i]))
            return false;
    }

    return i > hex_index;
}

bool isOct(const char *str) {
    if (!str || !*str) return false;
    const size_t oct_index = strlen(OCT_PREF);

    if (strncasecmp(str, OCT_PREF, oct_index) != 0)
        return false;

    uint16_t i = oct_index;
    for (; str[i]; i++) {
        if (str[i] < '0' || str[i] > '7')
            return false;
    }

    return i > oct_index;
}

int16_t strchar(const char *str, int8_t chr) {
    if (strlen(str) == 0) return -1;

    for (uint16_t i = 0; str[i]; i++) 
        if (str[i] == (char)chr) return i;

    return -1;   
}

void int64_to_hex_min(int64_t v, char *out, size_t size) {
    uint64_t u = (uint64_t)v;

    int32_t bits;
    for (bits = 8; bits < 64; bits++) {
        int64_t sign_bit = 1LL << (bits - 1);
        int64_t min = -sign_bit;
        int64_t max = sign_bit - 1;

        if (v >= min && v <= max)
            break;
    }

    int32_t hex_digits = (bits + 3) / 4;
    uint64_t mask;
    if (hex_digits == 16)
        mask = UINT64_MAX;
    else
        mask = (1ULL << (hex_digits * 4)) - 1;

    u &= mask;

    snprintf(out, size, "\""HEX_PREF"%0*"PRIX64"\"", hex_digits, u);
}

int64_t hex_to_long(char *str) {
    char *end;

    if (strncasecmp(str, HEX_PREF, strlen(HEX_PREF)) == 0)
        str += strlen(HEX_PREF);

    int64_t v = strtoll(str, &end, 16);

    if (*end != '\0')
        return U64_NAN;

    size_t digits = 0;
    for (char *p = str; *p; ++p)
        if (isxdigit(*p)) digits++;

    if (digits == 0)
        return 0;

    size_t bits = digits * 4;

    if (bits >= 64)
        return v;

    int64_t sign_bit = 1LL << (bits - 1);
    int64_t mask     = (1LL << bits) - 1;

    v &= mask;

    if (v & sign_bit)
        v -= (1LL << bits);

    return v;
}

void printc(const char *fmt, color4 initColor, color4 resetColor, ...) {
    setColor(initColor);

    va_list args;
    va_start(args, resetColor);
    vprintf(fmt, args);
    va_end(args);

    setColor(resetColor);
}

void setColor(color4 color) {
#ifdef _WIN64
    SetConsoleTextAttribute(hConsole, color);
#else
    switch(color) {
        case BLACK:           printf("\033[30m"); break;
        case BLUE:            printf("\033[34m"); break;
        case GREEN:           printf("\033[32m"); break;
        case CYAN:            printf("\033[36m"); break;
        case RED:             printf("\033[31m"); break;
        case MAGENTA:         printf("\033[35m"); break;
        case YELLOW:          printf("\033[33m"); break;
        case WHITE:           printf("\033[37m"); break;
        case GRAY:            printf("\033[90m"); break;
        case LIGHT_BLUE:      printf("\033[94m"); break;
        case LIGHT_GREEN:     printf("\033[92m"); break;
        case LIGHT_CYAN:      printf("\033[96m"); break;
        case LIGHT_RED:       printf("\033[91m"); break;
        case LIGHT_MAGENTA:   printf("\033[95m"); break;
        case LIGHT_YELLOW:    printf("\033[93m"); break;
        case BRIGHT_WHITE:    printf("\033[97m"); break;
        default:              printf("\033[0m");  break;
    }
#endif
}

void print_manual(void) {
    printf(INIT_MESSAGE);
    printf("\nUsage:\n\tbc [OPTION...]\n\nOptions:\n");
    printf("\t'-h', '--help'      displays manual\n");
    printf("\t'-v', '--version'   displays the version\n");
    printf("\t'-q', '--quiet'     makes it not print the initial message\n");
    printf("\t'-l', '--mathlib'   includes the mathlib header\n");
    printf("\nOperations:\n"
        "\t'+'    : Addition / string concatenation\n"
        "\t         Example: 2 + 3 = 5 / \"string1\" + \"string2\" = \"string1string2\"\n"
        "\n"
        "\t'-'    : Subtraction\n"
        "\t         Example: 10 - 4 = 6\n"
        "\n"
        "\t'*'    : Multiplication\n"
        "\t         Example: 6 * 7 = 42\n"
        "\n"
        "\t'/'    : Division\n"
        "\t         Example: 8 / 2 = 4\n"
        "\n"
        "\t'%%'    : Modulus (remainder of division)\n"
        "\t         Example: 10 %% 3 = 1\n"
        "\t         Note: it also works with single point precision numbers\n"
        "\n"
        "\t'^'    : Bitwise XOR\n"
        "\t         Example: 5 ^ 3 = 6\n"
        "\t         Explanation: Operates on individual bits\n"
        "\t         Explanation: Each bit is compared. Result bit is 1 if the bits are different\n"
        "\n"
        "\t'&'    : Bitwise AND\n"
        "\t         Example: 5 & 3 = 1\n"
        "\t         Explanation: Each bit is compared. Result bit is 1 only if both bits are 1\n"
        "\n"
        "\t'|'    : Bitwise OR\n"
        "\t         Example: 5 | 2 = 7\n"
        "\t         Explanation: Each bit is compared. Result bit is 1 if at least one bit is 1\n"
        "\n"
        "\t'~'    : Bitwise NOT\n"
        "\t         Example: ~2 = -3 and 100 ^ ~100 = -1\n"
        "\t         Explanation: Inverts every bit of the number\n"
        "\n"
        "\t'<<'   : Bitwise left shift\n"
        "\t         Example: 1 << 3 = 8\n"
        "\t         Explanation: 0b00001 << 3 = 0b01000\n"
        "\n"
        "\t'>>'   : Bitwise right shift\n"
        "\t         Example: 8 >> 2 = 2\n"
        "\t         Explanation: 0b01000 >> 2 = 0b00010, the bits were dislocated 2 times to the right\n"  
        "\n"
        "\t'!'    : Logical NOT\n"
        "\t         Example: !true = false / !false = true\n"
        "\t         Explanation: the inverse of true is false and the inverse of false is true\n"
        "\n"
        "\t'&&'   : Logical AND\n"
        "\t         Example: 5 && 0 = false\n"
        "\t         Note: Any non-zero value is treated as true\n"
        "\t         Explanation: Result is true(1) only if both operands are true(1), otherwise returns false(0)\n"
        "\n"
        "\t'||'   : Logical OR\n"
        "\t         Example: 0 || 5 = true\n"
        "\t         Explanation: Result is true(1) if at least one operand is true(1), otherwise returns false(0)\n"
        "\n"
        "\t'<'    : Less than\n"
        "\t         Example: 3 < 5 = true / \"apple\" < \"banana\" = true\n"
        "\t         Explanation: returns true(1) if the first value is less than the second, otherwise returns false(0)\n"
        "\n"
        "\t'<='   : Less than or equal\n"
        "\t         Example: 5 <= 5 = true / \"apple\" <= \"apple\" = true\n"
        "\t         Explanation: returns true(1) if the first value is less or equal than the second, otherwise returns false(0)\n"
        "\n"
        "\t'>'    : Greater than\n"
        "\t         Example: 8 > 3 = true / \"banana\" > \"apple\" = true\n"
        "\t         Explanation: returns true(1) if the first value is greater than the second, otherwise returns false(0)\n"
        "\n"
        "\t'>='   : Greater than or equal\n"
        "\t         Example: 4 >= 4 = true / \"banana\" >= \"banana\" = true\n"
        "\t         Explanation: returns true(1) if the first value is greater or equal than the second, otherwise returns false(0)\n"
        "\n"
        "\t'=='   : Equal to\n"
        "\t         Example: 6 == 6 = true / \"same\" == \"same\" = true\n"
        "\t         Explanation: returns true(1) if the first value is equal to the second, otherwise returns false(0)\n"
        "\n"
        "\t'!='   : Not equal to\n"
        "\t         Example: 6 != 5 = true / \"apple\" != \"banana\" = true\n"
        "\t         Explanation: returns true(1) if the first value is different from the second, otherwise returns false(0)\n"


        "\nFunctions: (mathlib must be on to grant access)\n"
        "\tscale(X)       : Sets decimal precision\n"
        "\t                 Example: scale(3.1415) = 4\n"
        "\t                 Explanation: Number of digits after decimal point\n"
        "\n"
        "\tsqrt(X)        : Square root\n"
        "\t                 Example: sqrt(16) = 4\n"
        "\n"
        "\troot(X, Y)     : X-th root of Y\n"
        "\t                 Example: root(3, 27) = 3\n"
        "\n"
        "\tsin(X)         : Sine of X\n"
        "\t                 Example: sin(rad(90)) = 1\n"
        "\t                 Note: X is in radians\n"
        "\n"
        "\tasin(X)        : Arc sine (inverse sine) of X\n"
        "\t                 Example: asin(0.5) = 0.523599\n"
        "\t                 Note: Returns value in radians\n"
    );
    printf(       
        "\n"
        "\tcos(X)         : Cosine of X\n"
        "\t                 Example: cos(rad(0)) = 1\n"
        "\t                 Note: X is in radians\n"
        "\n"
        "\tacos(X)        : Arc cosine (inverse cosine) of X\n"
        "\t                 Example: acos(0.5) = 1.0472\n"
        "\t                 Note: Returns value in radians\n"
        "\n"
        "\ttan(X)         : Tangent of X\n"
        "\t                 Example: tan(rad(45)) = 1\n"
        "\t                 Note: X is in radians\n"
        "\n"
        "\tatan(X)        : Arc tangent (inverse tangent) of X\n"
        "\t                 Example: atan(1) = 0.785398\n"
        "\t                 Note: Returns value in radians\n"
        "\n"
        "\tcot(X)         : Cotangent of X\n"
        "\t                 Example: cot(rad(30)) = 1.73205\n"
        "\t                 Note: X is in radians\n"
        "\n"
        "\tacot(X)        : Arc cotangent (inverse cotangent) of X\n"
        "\t                 Example: acot(1) = 0.785398\n"
        "\t                 Note: Returns value in radians (range: 0 < result < PI)\n"
        "\n"
        "\tdeg2rad(X)     : Degrees to radians\n"
        "\t                 Example: rad(3.1415) = 180\n"
        "\n"
        "\trad2deg(X)     : Radians to degrees\n"
        "\t                 Example: deg(180) = 3.1415\n"
        "\n"
        "\trad2gon(X)     : Radians to gradians\n"
        "\t                 Example: gon(1) = 63.662\n"
        "\n"
        "\tln(X)          : Natural logarithm\n"
        "\t                 Example: ln(E) = 1\n"
        "\n"
        "\tlog10(X)       : Base-10 logarithm\n"
        "\t                 Example: log10(1000) = 3\n"
        "\n"
        "\tlog2(X)        : Base-2 logarithm\n"
        "\t                 Example: log2(8) = 3\n"     
        "\n"
        "\tlog(X, Y)      : Logarithm of Y in base X\n"
        "\t                 Example: log(2, 32) = 5\n"
        "\n"
        "\tfloor(X)       : Rounds down\n"
        "\t                 Example: floor(3.7) = 3\n"
        "\n"
        "\tceil(X)        : Rounds up\n"
        "\t                 Example: ceil(3.2) = 4\n"
        "\n"
        "\tround(X)       : Rounds to nearest integer\n"
        "\t                 Example: round(3.5) = 4\n"
        "\n"
        "\tsum(X, Y, Z)   : Sum from X to Y with step Z\n"
        "\t                 Example: sum(1, 10, 2) = 25\n"
        "\t                 Note: If Z is omitted, step defaults to 1\n"
        "\n"
        "\ttrunc(X)       : Integer part of X\n"
        "\t                 Example: trunc(3.9) = 3\n"
        "\n"
        "\trand(X, Y)     : Random integer between X and Y\n"
        "\t                 Example: rand(1, 10)\n"
        "\n"
        "\trandf(X, Y)    : Random float between X and Y\n"
        "\t                 Example: randf(0, 1)\n"
        "\n"
        "\thex(X)         : Convert X to hexadecimal\n"
        "\t                 Example: hex(255) = "HEX_PREF"0FF\n"
        "\n"
        "\toct(X)         : Convert X to octal\n"
        "\t                 Example: oct(8) = "OCT_PREF"10\n"
        "\n"
        "\tbin(X)         : Convert X to binary\n"
        "\t                 Example: bin(5) = "BIN_PREF"0101\n"
    );
    printf(
        "\n"
        "\t"CHR_VAR"(X)         : Convert X to ascii\n"
        "\t                 Example: "CHR_VAR"(65) = 'A'\n"
        "\t                 Note: escape characters does not work\n"
        "\t                 Tip: requires an argument of type '"INT_VAR"' between 0 and 127 (inclusive)\n"
        "\n"
        "\t"BOOL_VAR"(X)        : Converts X to boolean\n"
        "\t                 Example: "BOOL_VAR"(0) = "FALSE_VAR" / "BOOL_VAR"(1) = "TRUE_VAR"\n"
        "\n"
        "\t"INT_VAR"(X)         : Converts X to integer\n"
        "\t                 Example: "INT_VAR"(\"2\") = 2\n"
        "\n"
        "\t"FLOAT_VAR"(X)       : Converts X to a floating point number\n"
        "\t                 Example: "FLOAT_VAR"(\"3.1415\") = 3.1415\n"
        "\n"
        "\t"STR_VAR"(X)         : Converts X to string\n"
        "\t                 Example: "STR_VAR"(PI) = \"3.14159\"\n"
        "\n"
        "\tfah(X)         : Celsius to Fahrenheit\n"
        "\t                 Example: fah(0) = 32\n"
        "\n"
        "\tcel(X)         : Fahrenheit to Celsius\n"
        "\t                 Example: cel(32) = 0\n"
        "\n"
        "\tmi(X)          : Kilometers to miles\n"
        "\t                 Example: mi(1) = 0.621\n"
        "\n"
        "\tkm(X)          : Miles to kilometers\n"
        "\t                 Example: km(1) = 1.609\n"
        "\n"
        "\tlb(X)          : Kilograms to pounds\n"
        "\t                 Example: lb(1) = 2.2046\n"
        "\n"
        "\tkg(X)          : Pounds to kilograms\n"
        "\t                 Example: kg(1) = 0.4535\n"
        "\n"
        "\tmeter(X)       : Converts feet (X) to meters\n"
        "\t                 Example: meter(5.8399) = 1.78\n"
        "\n"
        "\tfeet(X)        : Converts meter (X) to feet\n"
        "\t                 Example: feet(1.78) = 5.8399\n"
        "\n"
        "\tabs(X)         : Absolute value (int and float values)\n"
        "\t                 Example: abs(-7) = 7\n"
        "\n"
        "\tlen(str)       : Returns the length of a string\n"
        "\t                 Example: strlen(\"string\") = 6\n"
        "\n"
        "\tbmi(X, Y)      : Returns your BMI with wight (X) in kg and height (Y) in meters\n"
        "\t                 Example: bmi(91, 1.78) = 28.7211\n"
        "\n"
        "\tisprime(X)     : Returns 1 if X is prime, otherwise it returns 0\n"
        "\t                 Example: isprime(5) = 1\n"
        "\t                 Note: it requires an argument of type '"INT_VAR"' which is greater 1\n"
        "\n"
        "\tlower(X)       : Returns the string in lower case form\n"
        "\t                 Example: lower(\"STRING\") = \"string\"\n"
        "\n"
        "\tupper(X)       : Returns the string in upper case form\n"
        "\t                 Example: lower(\"string\") = \"STRING\"\n"
        "\n"
        "\ttypeof(X)      : Returns the type of the argument as a string\n"
        "\t                 Example: typeof(\"string\") = \""STR_VAR"\" / typeof(2) = \""INT_VAR"\"\n"
        "\n"
        "\ttet(X, Y)      : Tetration (X raised to itself Y times)\n"
        "\t                 Example: tet(2, 3) = 16\n"
        "\n"
        "\tpow(X, Y)      : Power (X multiplied to itself Y times)\n"
        "\t                 Example: pow(2, 3) = 8\n"
        "\n"
        "\tinput(X)       : Takes an input from the user and returns it as a string\n"
        "\t                 Example: input(\"Type-in: \") Type-in: <input typed> = \"input typed\"\n"

        "\nBuiltin Variables: (mathlib must be on to grant access)\n"
        "\tAns   : stores the result of the last operation\n"
        "\t        Tip: initially it is undefined\n"
        "\t        Note: its value cannot be changed manually\n"

        "\nConstants: (mathlib must be on to grant access)\n"
        "\t"PI_VAR"         : 3.141592...\n"
        "\t             Example: sin("PI_VAR" / 2) = 1\n"
        "\n"
        "\t"E_VAR"          : 2.718281...\n"
        "\t             Example: ln("E_VAR") = 1\n"
        "\n"
        "\t"INF_VAR"        : 1.797e+308 (64 bit)\n"
        "\t             Example: acot(-"INF_VAR") = "PI_VAR"\n"
        "\n"
        "\t"TRUE_VAR"       : 1 (boolean)\n"
        "\t             Example: (sen(deg2rad(30)) == sen(deg2rad(150))) = "TRUE_VAR"\n"
        "\n"
        "\t"FALSE_VAR"      : 0 (boolean)\n"
        "\t             Example: 1 != 5 = "FALSE_VAR"\n"
        "\n"
        "\t"RAND_MAX_VAR"   : "HEX_PREF"7FFFFFFF ((2 ** 31) - 1)\n"
        "\t             Explanation: stores the max value of a random function\n"
        "\n"

        "\nSuffixes:\n"
        "\t'!' : factorial\n"
        "\t      Example: 5! = 120 / 5!! = 15\n"

        "\nNumeric systems: (mathlib must be on to grant full access)\n"
        "\tBinary: (prefix: '"BIN_PREF"')        base 2 numbers e.g. "BIN_PREF"010000000000 = 1024\n"
        "\n"
        "\tDecimal: (default):           base 10 numbers e.g. 1024\n"
        "\n"
        "\tOctal: (prefix: '"OCT_PREF"')         base 8 numbers e.g. "OCT_PREF"2000 = 1024\n"
        "\n"
        "\tHexadecimal: (prefix: '"HEX_PREF"')   base 16 numbers e.g. "HEX_PREF"400 = 1024\n"
        "\n"
        "\tAscii: (characters)           1 bytes chars only e.g. 'a' = 97\n"
    );
}
