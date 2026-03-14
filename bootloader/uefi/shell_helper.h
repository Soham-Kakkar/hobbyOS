#pragma once

#ifndef SHELL_HELPER_H
#define SHELL_HELPER_H

#include "uefi_base.h"
#include "../../common/bootinfo.h"

/* ===============================
   Command Table
   =============================== */
typedef void (*shell_cmd_fn)(CHAR16* args);

typedef struct {
    const CHAR16* name;
    shell_cmd_fn  handler;
    const CHAR16* description;
} SHELL_COMMAND;

extern const UINTN COMMAND_COUNT;
extern SHELL_COMMAND command_table[];

/* ===============================
   Command Dispatcher
   =============================== */
void execute_command(CHAR16* line);

#endif // SHELL_HELPER_H