#include "includes/eval.h"

#ifdef _WIN64
    HANDLE hConsole;
#endif

#define BC_MATHLIB 0x0001
#define BC_QUIET   0x0002

int32_t main(int32_t argc, char **argv) {

    initRandom();

    char operation[MAX_CHAR];
    uint8_t flags = 0;

    bool mathlib = false;
    bool show_init = true;

    if (Ans.type == BC_STR && Ans.data.s)
        SAFE_FREE(Ans.data.s);
    else
        Ans.type = BC_NONE;

    bool has_expr = false;

    if (argc > 1) {
        for (uint16_t i = 1; i < argc; i++) {
            char *opt = argv[i];

            if (*opt == '-') {
                if (opt[1] == '-') {
                    if (strcmp(opt, "--mathlib") == 0)
                        flags |= BC_MATHLIB;
                    else if (strcmp(opt, "--quiet") == 0)
                        flags |= BC_QUIET;
                    else {
                        printc("eval", BC_PROMPT_COLOR, WHITE);
                        printf(": ");
                        printc("invalid option: '%s'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, opt);
                        return 1;
                    }
                } else {
                    for (uint16_t j = 1; opt[j]; j++) {
                        unsigned char chr = (unsigned char)opt[j];
                        switch (chr) {
                            case 'l': flags |= BC_MATHLIB; break;
                            case 'q': flags |= BC_QUIET; break;
                            default:
                                printc("eval", BC_PROMPT_COLOR, WHITE);
                                printf(": ");
                                printc("invalid option: '-%c'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, opt[j]);
                                return 1;
                        }
                    }
                }
            } else {
                has_expr = true;
            }
        }

        if (flags & BC_MATHLIB)
            mathlib = true;     

        if (flags & BC_QUIET)
            show_init = false;

        if (has_expr) {
            for (uint16_t i = 1; i < argc; i++) {
                char *opt = argv[i];

                if (*opt == '-') continue;

                char *buff = eval(opt, mathlib);

                if (!buff)
                    return 1;

                puts(buff);
                SAFE_FREE(buff);
            }

            return 0;
        }
    }

    char *result = NULL;
    bool appear = false;

    if (!isatty(STDIN_FILENO)) {
        char line[MAX_CHAR];

        while (fgets(line, sizeof(line), stdin)) {
            line[strcspn(line, "\n")] = '\0';

            if (!*line) continue;

            char *result = eval(line, mathlib);

            if (!result)
                return 1;

            puts(result);
            SAFE_FREE(result);
        }

        return 0;
    }

    while (true) {
        if (!appear) {
            if (show_init)
                printf("'ceval' a simple eval function implemented in C, operand precedence does not work unless if you use parenthesis, it support various types of operands.\n"
                        "type 'mathlib' if you're insite ceval to turn on the math library.\n"
                );
            if (mathlib)
                printc("on\n\n", GREEN, WHITE);
            else 
                printc("off\n\n", RED, WHITE);
        }

        appear = true;

        printc(">>> ", BC_PROMPT_COLOR, WHITE);
        fgets(operation, sizeof(operation), stdin);
        operation[strcspn(operation, "\n")] = '\0';

        if (!*operation) {
            putchar('\n');
            continue;
        }

        if (isValidBcCommand(operation, "quit") ||
            isValidBcCommand(operation, "exit")) {
            break;

        } else if (isValidBcCommand(operation, "mathlib")) {
            clear();
            mathlib = !mathlib;
            appear = false;
            continue;
        } else if (isValidBcCommand(operation, "quiet")) {
            clear();
            show_init = !show_init;
            appear = false;
            continue;
        } else if (isValidBcCommand(operation, "cls")) {
            clear();
            appear = false;
            continue;
        } else if (isValidBcCommand(operation, "clear")) {
            clear();
            appear = false;
            continue;
        } 

        result = eval(operation, mathlib);

        if (!result) {
            putchar('\n');
            SAFE_FREE(result);
            fflush(stdout);
            continue;
        }

        printf("%s\n\n", result);

        SAFE_FREE(result);
        fflush(stdout);
    }

    return 0;
}