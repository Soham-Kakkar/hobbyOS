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

static EFI_FILE* open_path(EFI_FILE* root, CHAR16* path) {
    EFI_FILE* current = root;
    EFI_FILE* next;

    CHAR16* p = path;
    while (*p) {

        /* find next separator */
        CHAR16* start = p;
        while (*p && *p != L'/' && *p != L'\\') p++;

        CHAR16 saved = *p;
        *p = 0;

        EFI_STATUS status = current->Open(current, &next, start, EFI_FILE_MODE_READ, 0);

        *p = saved;

        if (status != EFI_SUCCESS) {
            return NULL;
        }

        current = next;

        if (*p) p++;
    }

    return current;
}

/* ===============================
   Commands
   =============================== */

static void cmd_help(CHAR16* args) {
    (void)args;

    Print(L"Welcome to HobbyOS bootloader shell.\r\n");
    Print(L"Commands:\r\n");

    for (UINTN i = 0; i < COMMAND_COUNT; i++) {
        Print(L"  %s - %s\r\n",
              command_table[i].name,
              command_table[i].description);
    }
}

static void cmd_clear(CHAR16* args) {
    (void)args;
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

static void cmd_memmap(CHAR16* args) {
    (void)args;

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
    Print(L"Idx  Type               Start          Pages        Space (KB)\r\n");
    Print(L"--------------------------------------------------------------\r\n");

    for (UINTN i = 0; i < entries; i++) {

        EFI_MEMORY_DESCRIPTOR *desc =
            (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)map + i * desc_size);

        Print(L"%u    %s    %lx %lu %lu\r\n",
              i,
              efi_mem_type_str(desc->Type),
              desc->PhysicalStart,
              desc->NumberOfPages,
              (desc->NumberOfPages * PAGE_SIZE) / 1024);
    }

    FreePool(map);
}

static void cmd_gopinfo(CHAR16* args) {
    (void)args;

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

static void cmd_ls(CHAR16* args) {
    EFI_FILE *root = open_root(gImageHandle, gST);

    if (!root) {
        Print(L"Failed to open root\r\n");
        return;
    }

    EFI_FILE *dir = root;

    if (args && *args) {

        dir = open_path(root, args);

        if (!dir) {
            Print(L"Cannot open: %s\r\n", args);
            return;
        }
    }

    dir->SetPosition(dir, 0);

    UINTN buf_size = SIZE_OF_EFI_FILE_INFO + 256;
    EFI_FILE_INFO *info = AllocatePool(buf_size);

    if (!info) {
        Print(L"Out of memory\r\n");
        return;
    }

    Print(L"Files:\r\n");

    while (1) {
        UINTN size = buf_size;

        EFI_STATUS status = dir->Read(dir, &size, info);

        if (status != EFI_SUCCESS || size == 0)
            break;

        if (info->Attribute & EFI_FILE_DIRECTORY) {
            Print(L"[DIR]  %s\r\n", info->FileName);
        } else {
            Print(L"       %s (%lu bytes)\r\n", info->FileName, info->FileSize);
        }
    }

    FreePool(info);
}

/* ===============================
   Command Table
   =============================== */

    SHELL_COMMAND command_table[] = {
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