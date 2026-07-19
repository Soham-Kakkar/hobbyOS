#pragma once

#ifndef CMD_LOADKERNEL_H
#define CMD_LOADKERNEL_H

#include "../../uefi_base/uefi_base.h"
#include "../elf64.h"
#include "../helpers.h"
#include "../../../../common/bootinfo.h"

void cmd_loadkernel(CHAR16* args);

#endif // CMD_LOADKERNEL_H