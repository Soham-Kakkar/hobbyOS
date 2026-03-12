#include "uefi_base.h"
#include "../../common/bootinfo.h"
#include "shell_helper.h"

/* ===============================
   Helpers
   =============================== */

static int str_eq(const CHAR16* a, const CHAR16* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return (*a == 0 && *b == 0);
}

static int split_cmd(CHAR16* line, CHAR16** cmd, CHAR16** args) {
    *cmd = line;
    *args = 0;

    while (*line) {
        if (*line == L' ') {
            *line = 0;
            *args = line + 1;
            return 1;
        }
        line++;
    }

    return 0;
}

/* ===============================
   Commands
   =============================== */

static void cmd_help(void) {
    Print(L"Welcome to HobbyOS bootloader shell.\r\n");
    Print(L"Supported commands:\r\n");
    Print(L"  help    - show this help\r\n");
    Print(L"  clear   - clear the screen\r\n");
    Print(L"  print   - print text\r\n");
    Print(L"  memmap  - show memory map\r\n");
    Print(L"  gopinfo - show Graphics Output Protocol info\r\n");
}

static void cmd_clear(CHAR16* args) {
    clear();
}
static void cmd_print(CHAR16* args) {
    if (!args) {
        Print(L"(nothing to print)\r\n");
        return;
    }

    Print(args);
    Print(L"\r\n");
}

static const CHAR16* efi_mem_type_str(UINT32 type) {
    switch (type) {
        case 0: return L"Reserved";
        case 1: return L"LoaderCode";
        case 2: return L"LoaderData";
        case 3: return L"BootServicesCode";
        case 4: return L"BootServicesData";
        case 5: return L"RuntimeServicesCode";
        case 6: return L"RuntimeServicesData";
        case 7: return L"Conventional";
        case 8: return L"Unusable";
        case 9: return L"ACPIReclaim";
        case 10: return L"ACPIMemoryNVS";
        case 11: return L"MemoryMappedIO";
        case 12: return L"MemoryMappedIOPortSpace";
        case 13: return L"PalCode";
        default: return L"Unknown";
    }
}

static void cmd_memmap(void) {
    EFI_MEMORY_DESCRIPTOR *map = NULL;
    UINTN map_size = 0;
    UINTN map_key = 0;
    UINTN desc_size = 0;
    UINT32 desc_version = 0;

    EFI_STATUS status = get_memory_map(
        &map,
        &map_size,
        &map_key,
        &desc_size,
        &desc_version
    );

    if (status != EFI_SUCCESS) {
        Print(L"Failed to get memory map: %x\r\n", status);
        return;
    }

    UINTN entries = map_size / desc_size;

    Print(L"Memory Map (%u descriptors)\r\n\r\n", entries);
    Print(L"Idx  Type               Start              Pages\r\n");
    Print(L"------------------------------------------------\r\n");

    for (UINTN i = 0; i < entries; i++) {

        EFI_MEMORY_DESCRIPTOR *desc =
            (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)map + i * desc_size);

        Print(L"%u    %s    %x    %u\r\n",
              i,
              efi_mem_type_str(desc->Type),
              desc->PhysicalStart,
              desc->NumberOfPages);
    }

    gBS->FreePool(map);
}

static void cmd_gopinfo(void)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

    EFI_STATUS status =
        gBS->LocateProtocol(
            &gEfiGraphicsOutputProtocolGuid,
            NULL,
            (void**)&gop
        );

    if (status != EFI_SUCCESS) {
        Print(L"GOP not available\r\n");
        return;
    }

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info = gop->Mode->Info;

    Print(L"Graphics Output Protocol\r\n\r\n");

    Print(L"Resolution: %u x %u\r\n",
          info->HorizontalResolution,
          info->VerticalResolution);

    Print(L"PixelsPerScanLine: %u\r\n",
          info->PixelsPerScanLine);

    Print(L"PixelFormat: %u\r\n",
          info->PixelFormat);

    Print(L"\r\nFramebuffer Base: %x\r\n",
          gop->Mode->FrameBufferBase);

    Print(L"Framebuffer Size: %u bytes\r\n",
          gop->Mode->FrameBufferSize);
}

/* ===============================
   Command Dispatcher
   =============================== */

void execute_command(CHAR16* line) {
    CHAR16* cmd;
    CHAR16* args;

    split_cmd(line, &cmd, &args);

    if (str_eq(cmd, L"help")) {
        cmd_help();
        return;
    }

    if (str_eq(cmd, L"print")) {
        cmd_print(args);
        return;
    }

    if (str_eq(cmd, L"memmap")) {
        cmd_memmap();
        return;
    }

    if (str_eq(cmd, L"gopinfo")) {
        cmd_gopinfo();
        return;
    }

    if (str_eq(cmd, L"clear")) {
        cmd_clear(args);
        return;
    }

    Print(L"Unknown command\r\n");
}