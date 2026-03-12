#pragma once

#ifndef SHELL_HELPER_H
#define SHELL_HELPER_H

#include "uefi_base.h"
#include "../../common/bootinfo.h"

/* ===============================
   Command Dispatcher
   =============================== */
void execute_command(CHAR16* line);

#endif // SHELL_HELPER_H