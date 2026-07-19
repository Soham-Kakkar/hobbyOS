#pragma once

#ifndef CMD_HELPERS_H
#define CMD_HELPERS_H

#include <efi.h>

int str_eq(const CHAR16* a, const CHAR16* b);
int split_cmd(CHAR16* line, CHAR16** cmd, CHAR16** args);
EFI_FILE* open_path(EFI_FILE* root, CHAR16* path);

#endif // CMD_HELPERS_H