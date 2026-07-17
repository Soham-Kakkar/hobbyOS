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

    Print(L" _   _       _      ____  _          _ _ \r\n");
    Print(L"| | | | ___ | |__  / ___|| |__   ___| | |\r\n");
    Print(L"| |_| |/ _ \\| '_ \\ \\___ \\| '_ \\ / _ \\ | |\r\n");
    Print(L"|  _  | (_) | |_) | ___) | | | |  __/ | |\r\n");
    Print(L"|_| |_|\\___/|_.__/ |____/|_| |_|\\___|_|_|\r\n\r\n");

    Print(L"Welcome to HobbyOS bootloader shell.\r\n");
    Print(L"Type 'help' for a list of commands.\r\n\r\n");

    while (1) {

        Print(L"boot>");

        int len = readline(input, INPUT_MAX);

        if (len <= 0)
            continue;

        execute_command(input);
    }

    return EFI_SUCCESS;
}