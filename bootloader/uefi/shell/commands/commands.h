#pragma once

#include "../helpers.h"
#include "../../uefi_base/uefi_base.h"

void cmd_print(CHAR16* args);
void cmd_clear(CHAR16* args);
void cmd_help(CHAR16* args);
void cmd_ls(CHAR16* args);
void cmd_memmap(CHAR16* args);
void cmd_gopinfo(CHAR16* args);

#ifdef WITH_LOADKERNEL
void cmd_loadkernel(CHAR16* args);
#endif