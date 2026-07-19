#include "commands.h"
#include "../shell.h"

void cmd_print(CHAR16* args) {
    if (!args) {
        Print(L"(nothing to print)\r\n");
        return;
    }

    Print(args);
    Print(L"\r\n");
}

void cmd_clear(CHAR16* args) {
    (void)args;
    clear();
}

void cmd_help(CHAR16* args) {
    (void)args;

    Print(L"Welcome to HobbyOS bootloader shell.\r\n");
    Print(L"Commands:\r\n");

    for (UINTN i = 0; i < COMMAND_COUNT; i++) {
        Print(L"  %s - %s\r\n",
              command_table[i].name,
              command_table[i].description);
    }
}

void cmd_ls(CHAR16* args) {
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
            root->Close(root);
            return;
        }
    }

    dir->SetPosition(dir, 0);

    UINTN buf_size = SIZE_OF_EFI_FILE_INFO + 256;
    EFI_FILE_INFO *info = AllocatePool(buf_size);

    if (!info) {
        Print(L"Out of memory\r\n");
        if (dir != root) dir->Close(dir);
        root->Close(root);
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

    if (dir != root)
        dir->Close(dir);

    root->Close(root);
}

void cmd_gopinfo(CHAR16* args) {
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

void cmd_memmap(CHAR16* args) {
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