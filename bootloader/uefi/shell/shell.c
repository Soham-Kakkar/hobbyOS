#include "shell.h"

/* ===============================
   Command Table
   =============================== */

SHELL_COMMAND command_table[] = {
    #ifdef WITH_LOADKERNEL
    { L"load", cmd_loadkernel, L"load and execute kernel" },
    #endif
    { L"help",    cmd_help,    L"show this help" },
    { L"clear",   cmd_clear,   L"clear the screen" },
    { L"ls",      cmd_ls,      L"list files" },
    { L"print",   cmd_print,   L"print text" },
    { L"memmap",  cmd_memmap,  L"show memory map" },
    { L"gopinfo", cmd_gopinfo, L"show Graphics Output Protocol info" },
};

const UINTN COMMAND_COUNT = sizeof(command_table) / sizeof(command_table[0]);

/* ===============================
   Command Dispatcher
   =============================== */

void execute_command(CHAR16* line) {
    CHAR16* cmd;
    CHAR16* args;

    split_cmd(line, &cmd, &args);

    for (UINTN i = 0; i < COMMAND_COUNT; i++) {

        if (str_eq(cmd, command_table[i].name)) {
            command_table[i].handler(args);
            return;
        }
    }

    Print(L"Unknown command\r\n");
}

/* ===============================
   Shell
   =============================== */

#define INPUT_MAX 128
CHAR16 input[INPUT_MAX];

void shell_init(void) {
    while (1) {
        Print(L"boot>");

        int len = readline(input, INPUT_MAX);

        if (len <= 0)
            continue;

        execute_command(input);
    }
}
