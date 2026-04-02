#include "utils.h"

#ifdef _WIN64
    extern HANDLE hConsole;
#endif

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

void num_snprintf(char *buff, size_t size, double num) {
    if (T_CMP(num, (int64_t)num)) {
        snprintf(buff, size, "%" PRId64, (int64_t)num);
    } else {
        snprintf(buff, size, "%lf", num);

        size_t end = strlen(buff) - 1;

        while (end > 0 && buff[end] == '0') {
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

    if (isBetweenQuotes(str1, 1)) {
        size_t len = strlen(str1) - 2;
        strncpy(buff1, str1+1, len);
        buff1[len] = '\0';
    } else
        strncpy(buff1, str1, sizeof(buff1)-1), buff1[sizeof(buff1)-1] = '\0';

    if (isBetweenQuotes(str2, 1)) {
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
                color4 color1 = (strcasecmp(error_str, "eval") == 0) ? BC_PROMPT_COLOR : WHITE;
                color4 color2 = (color1 == BC_PROMPT_COLOR) ? GET_BASE_COLOR(color1) : WHITE;

                printc("%s", color1, WHITE, error_str);
                printf(": ");
                printc("missing escape char after slash\n", color2, WHITE);

                return 0;
            }

            char chr = str[i+1];

            if (!isIn(chr, "ntbra'\"?fv0\\")) {
                color4 color1 = (strcasecmp(error_str, "eval") == 0) ? BC_PROMPT_COLOR : WHITE;
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

int16_t find_main_operator_full(const char *s, const char **multiOps, const char *uniOps, char *foundOp) {
    int32_t len = strlen(s);

    int32_t depth = 0;
    bool in_single = false;
    bool in_double = false;

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

        if (c == ')') {
            depth++;
            continue;
        }
        else if (c == '(') {
            depth--;
            continue;
        }

        if (depth != 0)
            continue;

        for (int32_t j = 0; multiOps[j]; j++) {

            int32_t oplen = strlen(multiOps[j]);
            int32_t start = i - oplen + 1;

            if (start < 0)
                continue;

            if (strncmp(&s[start], multiOps[j], oplen) == 0) {

                strcpy(foundOp, multiOps[j]);
                return start;
            }
        }

        if (strchr(uniOps, c)) {

            int32_t k = i - 1;

            while (k >= 0 && isspace((unsigned char)s[k]))
                k--;

            if (k < 0)
                continue;

            bool is_prev_operator = false;

            if (strchr(uniOps, s[k]) || s[k] == '(') {
                is_prev_operator = true;
            } else {
                for (int16_t m = 0; multiOps[m]; m++) {
                    int16_t len = strlen(multiOps[m]);
                    if (k - len + 1 >= 0 &&
                        strncmp(&s[k - len + 1], multiOps[m], len) == 0) {
                        is_prev_operator = true;
                        break;
                    }
                }
            }

            if (is_prev_operator)
                continue;

            foundOp[0] = c;
            foundOp[1] = '\0';
            return i;
        }
    }

    return -1;
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

bool isIn(char needle, char *haystack) {
    for (size_t i = 0; haystack[i]; i++)
        if (needle == haystack[i])
            return true;

    return false;
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

double parse_base_fraction(const char *s, int8_t base) {
    double result = 0.0;
    double frac = 0.0;
    double div = base;
    int8_t seen_dot = 0;

    for (; *s; s++) {
        if (*s == '.') {
            if (seen_dot) break;
            seen_dot = 1;
            continue;
        }

        int32_t digit;
        if (*s >= '0' && *s <= '9') digit = *s - '0';
        else if (*s >= 'a' && *s <= 'f') digit = *s - 'a' + 10;
        else if (*s >= 'A' && *s <= 'F') digit = *s - 'A' + 10;
        else break;

        if (digit >= base) break;

        if (!seen_dot) {
            result = result * base + digit;
        } else {
            frac += digit / div;
            div *= base;
        }
    }

    return result + frac;
}

double parse_bin_hex_oct_ans_e_pi(const char *str, int16_t *ok) {
    *ok = 0;

    if (!str || !*str)
        return 0.0;

    char *cpy = strdup(str);
    charRm(cpy, ' ');

    int16_t sign = 1;
    int16_t pos = 0;
    int32_t base = 0;

    if (cpy[pos] == '-') {
        sign = -1;
        pos++;
    } else if (cpy[pos] == '+') {
        pos++;
    }

    bool isBinary = false; 
    if (strncasecmp(cpy + pos, HEX_PREF, strlen(HEX_PREF)) == 0) {
        base = 16;
        pos += 2;
    } else if (strncasecmp(cpy + pos, BIN_PREF, strlen(BIN_PREF)) == 0) {
        base = 2;
        pos += 2;
    } else if (strncasecmp(cpy + pos, OCT_PREF, strlen(OCT_PREF)) == 0) {
        base = 8;
        pos += 2;
    } else
        base = 10;

    int16_t num_start = pos;

    bool dot_seen = false;

    while (cpy[pos]) {
        if (cpy[pos] == '.') {
            if (dot_seen) break;
            dot_seen = true;
            pos++;
            continue;
        }

        int32_t digit;
        if (cpy[pos] >= '0' && cpy[pos] <= '9')
            digit = cpy[pos] - '0';
        else if (cpy[pos] >= 'a' && cpy[pos] <= 'f')
            digit = cpy[pos] - 'a' + 10;
        else if (cpy[pos] >= 'A' && cpy[pos] <= 'F')
            digit = cpy[pos] - 'A' + 10;
        else
            break;

        if (digit >= base)
            break;

        pos++;
    }

    if (pos == num_start) {
        SAFE_FREE(cpy);
        return 0.0;
    }

    double mult = 0.0;

    if (strcmp(cpy + pos, PI_VAR) == 0)
        mult = PI;
    else if (strcmp(cpy + pos, E_VAR) == 0)
        mult = E;
    else if (strcmp(cpy + pos, ANS_VAR) == 0) {
        if (Ans.type == BC_NONE) {
            printc("eval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("'ans' is undefined\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            return NAN;
        }

        if (Ans.type == BC_STR) {
            printc("eval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("cannot make juxtapositions operations with strings\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

            return NAN;
        }

        switch (Ans.type) {
            case BC_BOOL:
                mult = (double)Ans.data.b;
                break;
            case BC_CHR:
            case BC_INT:
                mult = (double)Ans.data.i;
                break;
            case BC_FLOAT:
                mult = Ans.data.f;
                break;
            default:
                return NAN;
        }
    } else {
        SAFE_FREE(cpy);
        return 0.0;
    }

    char buf[0x40];
    size_t len = pos - num_start;

    if (len >= sizeof(buf)) {   
        SAFE_FREE(cpy);
        return 0.0;
    }

    strncpy(buf, cpy + num_start, len);
    buf[len] = '\0';

    double value;

    if (!isBinary)
        value = parse_base_fraction(buf, base);
    else
        value = parse_base_fraction(buf, 2);

    *ok = 1;
    
    SAFE_FREE(cpy);
    return sign * value * mult;
}

bool isBcVariable(const char *str, bool *shouldError) {
    *shouldError = true;

    if (isBetweenQuotes(str, 2)) {
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

bool isBetweenQuotes(const char *action, int16_t quoteMode) {

    if (!action)
        return false;

    if (quoteMode < 0 || quoteMode > 2)
        return false;

    size_t len = strlen(action);

    if (len < 2)
        return false;

    char first = *action;
    char last  = action[len-1];

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

    size_t end = strlen(str) - 1;
    while (isIn(str[end], "kmbt") || isIn(str[end], "KMBT")) end --;

    const size_t bin_index = strlen(BIN_PREF);

    if (strncasecmp(str, BIN_PREF, bin_index) != 0)
        return false;

    for (uint16_t i = bin_index; i <= end; i++) {
        if (str[i] != '1' && str[i] != '0')
            return false;
    }

    return end >= strlen(BIN_PREF);
}

bool isHex(const char *str) {
    if (!str || !*str) return false;

    const size_t hex_index = strlen(HEX_PREF);

    if (strncasecmp(str, HEX_PREF, hex_index) != 0)
        return false;

    if (!str[hex_index])
        return false;

    for (size_t i = hex_index; str[i]; i++) {
        if (!isxdigit((unsigned char)str[i]))
            return false;
    }

    return true;
}

bool isOct(const char *str) {
    if (!str || !*str) return false;

    size_t end = strlen(str) - 1;
    while (isIn(str[end], "kmbt") || isIn(str[end], "KMBT")) end --;

    const size_t oct_index = strlen(OCT_PREF);

    if (strncasecmp(str, OCT_PREF, oct_index) != 0)
        return false;

    for (uint16_t i = oct_index; i <= end; i++) {
        if (str[i] < '0' || str[i] > '7')
            return false;
    }

    return end >= strlen(OCT_PREF);
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

void printc(const char *str, color4 initColor, color4 resetColor, ...) {
    setColor(initColor);

    va_list args;
    va_start(args, resetColor);
    vprintf(str, args);
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