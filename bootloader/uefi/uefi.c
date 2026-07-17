#include "uefi_base.h"
#include "shell_helper.h"

#define INPUT_MAX 128

CHAR16 input[INPUT_MAX];

/* ===============================
   Main Loop
   =============================== */

EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE* st) {
    InitializeLib(image, st);

    clear();

    Print(L"Welcome to HobbyOS bootloader shell.\r\n");

    while (1) {

        Print(L"boot>");

        int len = readline(input, INPUT_MAX);

        if (len <= 0)
            continue;

        execute_command(input);
    }

    return EFI_SUCCESS;
}