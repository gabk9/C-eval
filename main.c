#include "includes/utils.h"
#include "includes/eval.h"
#include "includes/types.h"

#ifdef _WIN64
    HANDLE hConsole;
#endif

bool mathlib = false;

#define BC_MATHLIB 0x0001
#define BC_QUIET   0x0002
#define BC_VERSION 0x0004
#define BC_HELP    0x0008

int32_t main(int32_t argc, char **argv) {
#ifdef _WIN64
    SetUnhandledExceptionFilter(handler);
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

    initRandom();

    char operation[INPUT_SIZE];
    uint8_t flags = 0;
    uint32_t objCount = 0;

    bool show_init = true;

    if (Ans.type == BC_STR && Ans.data.s) {
        SAFE_FREE(Ans.data.s);
        Ans.type = BC_NULL;
    } else
        Ans.type = BC_NULL;

    if (argc > 1) {
        for (int32_t i = 1; i < argc; i++) {
            char *opt = argv[i];

            if (*opt == '-') {
                if (opt[1] == '-') {
                    if (strcmp(opt, "--mathlib") == 0)
                        flags |= BC_MATHLIB;
                    else if (strcmp(opt, "--quiet") == 0)
                        flags |= BC_QUIET;
                    else if (strcmp(opt, "--version") == 0)
                        flags |= BC_VERSION;
                    else if (strcmp(opt, "--help") == 0)
                        flags |= BC_HELP;
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
                            case 'v': flags |= BC_VERSION; break;
                            case 'q': flags |= BC_QUIET; break;
                            case 'l': flags |= BC_MATHLIB; break;
                            case 'h': flags |= BC_HELP; break;
                            default:
                                printc("eval", BC_PROMPT_COLOR, WHITE);
                                printf(": ");
                                printc("invalid option: '-%c'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, opt[j]);
                                return 1;
                        }
                    }
                }
            } else
                argv[objCount++] = opt;
        }

        bool did_special = false;

        if (flags & BC_MATHLIB)
            mathlib = true;     

        if (flags & BC_QUIET) 
            show_init = false;

        if (flags & BC_VERSION) {
            puts(VERSION);
            did_special = true;
        }

        if (flags & BC_HELP) {
            print_manual();
            did_special = true;
        }

        if (objCount > 0) {
            for (uint32_t i = 0; i < objCount; i++) {
                if (!extractFilePath(argv[i])) {
                    printc("ceval", BC_PROMPT_COLOR, WHITE);
                    printf(": ");
                    printc("missing file argument!\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

                    return 1;
                }

                FILE *f = fopen(argv[i], "r");

                if (!f) {
                    printc("ceval", BC_PROMPT_COLOR, WHITE);
                    printf(": ");
                    printc("Cannot open '%s' No such file or directory\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, argv[i]);

                    return 1;
                }

                char line[MAX_CHAR] = {0};

                while (fgets(line, sizeof(line), f)) {
                    line[strcspn(line, "\r\n")] = '\0';

                    if (isEmpty(line))
                        continue;

                    char *fullLine = lineContinuation(line, f);

                    if (!fullLine) {
                        SAFE_FCLOSE(f);
                        return 1;
                    }

                    #ifdef INPUT_DEBUG
                        printc("line", BC_PROMPT_COLOR, WHITE);
                        printf(": ");
                        printc("'", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                        printf("%s", fullLine);
                        printc("'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
                    #endif

                    var debug = eval(fullLine);

                    SAFE_FREE(fullLine);

                    switch (debug.type) {
                        case BC_NULL:
                            SAFE_FCLOSE(f);
                            return 1;
                        case BC_STR:
                            SAFE_FREE(debug.data.s);
                            break;
                        default:
                            continue;
                    }
                }

                SAFE_FCLOSE(f);
            }

            return 0;
        }

        if (did_special)
            return 0;
    }

    char *result = NULL;
    bool appear = false;

    if (!isatty(STDIN_FILENO)) {
        printc("ceval", BC_PROMPT_COLOR, WHITE);
        printf(": ");
        printc("does not support tty input\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);

        return 1;
    }

    while (true) {
        if (!appear) {
            if (show_init)
                printf(INIT_MESSAGE);
            if (mathlib)
                printc("on\n\n", GREEN, WHITE);
            else 
                printc("off\n\n", RED, WHITE);
        }

        appear = true;

        printc(">>> ", BC_PROMPT_COLOR, WHITE);
        fgets(operation, sizeof(operation), stdin);

        if (!strchr(operation, '\n')) {
            int32_t c;
            while ((c = getchar()) != '\n' && c != EOF);

            printc("ceval", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("input too large (max %d characters)\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE, MAX_CHAR);
            continue;
        }

        operation[strcspn(operation, "\r\n")] = '\0';

        if (isEmpty(operation)) {
            putchar('\n');
            continue;
        }

        trimTabs(operation);

        char *new = lineContinuation(operation, stdin);

        if (!new) {
            putchar('\n');
            continue;
        }

        #ifdef INPUT_DEBUG
            printc("input", BC_PROMPT_COLOR, WHITE);
            printf(": ");
            printc("'", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
            printf("%s", new);
            printc("'\n", GET_BASE_COLOR(BC_PROMPT_COLOR), WHITE);
        #endif

        if (!*new) {
            putchar('\n');
            continue;
        }

        if (trim_streq(new, "quit") ||
            trim_streq(new, "exit")) {
            break;

        } else if (trim_streq(new, "mathlib")) {
            clear();
            mathlib = !mathlib;
            appear = false;
            continue;
        } else if (trim_streq(new, "cls")) {
            clear();
            appear = false;
            continue;
        } else if (trim_streq(new, "clear")) {
            clear();
            appear = false;
            continue;
        } else if (trim_streq(new, "help")) {
            print_manual();
            putchar('\n');
            appear = true;
            continue;
        }

        result = var2str(eval(new));

        if (!result) {
            putchar('\n');
            SAFE_FREE(result);
            fflush(stdout);
            continue;
        }

        printf("%s\n\n", result);

        SAFE_FREE(result);
        SAFE_FREE(new);
        fflush(stdout);
    }

    return 0;
}
