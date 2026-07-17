#include "uefi_base.h"
#include "../../common/bootinfo.h"
#include "shell_helper.h"
#include "elf64.h"

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
    int first = 1;

    while (*p) {

        /* find next separator */
        CHAR16* start = p;
        while (*p && *p != L'/' && *p != L'\\') p++;

        CHAR16 saved = *p;
        *p = 0;

        EFI_STATUS status = current->Open(current, &next, start, EFI_FILE_MODE_READ, 0);

        *p = saved;

        if (status != EFI_SUCCESS) {
            if (!first) current->Close(current);
            return NULL;
        }

        // Close previous handle (except root)
        if (!first) {
            current->Close(current);
        }

        current = next;
        first = 0;

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

static void cmd_loadkernel(CHAR16* args) {
    if (!args || !*args) {
        Print(L"Usage: load <kernel.elf>\r\n");
        return;
    }

    Print(L"[loader] opening kernel: %s\r\n", args);

    /* ===============================
     * Open + read ELF
     * =============================== */

    EFI_FILE *root = open_root(gImageHandle, gST);
    if (!root) return;

    EFI_FILE *file = open_path(root, args);
    if (!file) {
        root->Close(root);
        return;
    }

    void *kernel_buf;
    UINTN kernel_size;

    if (read_file(file, &kernel_buf, &kernel_size) != EFI_SUCCESS) {
        file->Close(file);
        root->Close(root);
        return;
    }

    file->Close(file);
    root->Close(root);

    Elf64_Ehdr *ehdr = (Elf64_Ehdr*)kernel_buf;

    if (ehdr->magic != ELF_MAGIC) {
        Print(L"[loader] bad ELF magic\r\n");
        return;
    }

    Elf64_Phdr *phdrs = (Elf64_Phdr*)((UINT8*)kernel_buf + ehdr->phoff);

    /* ===============================
     * Compute memory range
     * =============================== */

    UINT64 min_vaddr = (UINT64)-1;
    UINT64 max_vaddr = 0;

    for (UINTN i = 0; i < ehdr->phnum; i++) {
        Elf64_Phdr *ph = &phdrs[i];
        if (ph->type != PT_LOAD) continue;

        if (ph->vaddr < min_vaddr)
            min_vaddr = ph->vaddr;

        UINT64 end = ph->vaddr + ph->memsz;
        if (end > max_vaddr)
            max_vaddr = end;
    }

    UINT64 total_size = max_vaddr - min_vaddr;
    UINTN pages = (total_size + 0xFFF) / 0x1000;

    /* ===============================
     * Allocate contiguous kernel memory
     * =============================== */

    UINT64 kernel_base = 0;

    EFI_STATUS status = gBS->AllocatePages(
        AllocateAnyPages,
        EfiLoaderData,
        pages,
        &kernel_base
    );

    if (status != EFI_SUCCESS) {
        Print(L"[loader] kernel memory allocation failed\r\n");
        for (;;) __asm__("hlt");
    }

    Print(L"[loader] kernel base: %lx\r\n", kernel_base);

    /* ===============================
     * Load segments
     * =============================== */

    for (UINTN i = 0; i < ehdr->phnum; i++) {
        Elf64_Phdr *ph = &phdrs[i];
        if (ph->type != PT_LOAD) continue;

        UINT64 dest = kernel_base + (ph->vaddr - min_vaddr);

        memcpy((void*)dest,
               (UINT8*)kernel_buf + ph->offset,
               ph->filesz);

        if (ph->memsz > ph->filesz) {
            memset((void*)(dest + ph->filesz),
                   0,
                   ph->memsz - ph->filesz);
        }

        Print(L"[loader] segment %u → %lx\r\n", i, dest);
    }

    /* ===============================
     * GOP
     * =============================== */

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    if (gBS->LocateProtocol(
            &gEfiGraphicsOutputProtocolGuid,
            NULL,
            (void**)&gop) != EFI_SUCCESS) {
        Print(L"[loader] GOP not available\r\n");
        for (;;) __asm__("hlt");
    }

    /* ===============================
     * Allocate BootInfo
     * =============================== */

    BootInfo* bi;
    gBS->AllocatePool(EfiLoaderData, sizeof(BootInfo), (void**)&bi);

    bi->framebuffer = (uint32_t*)gop->Mode->FrameBufferBase;
    bi->width       = gop->Mode->Info->HorizontalResolution;
    bi->height      = gop->Mode->Info->VerticalResolution;
    bi->pitch       = gop->Mode->Info->PixelsPerScanLine;

    /* ===============================
     * Allocate kernel stack (CRITICAL)
     * =============================== */

    #define KERNEL_STACK_SIZE 0x40000

    UINT64 kernel_stack;
    gBS->AllocatePages(
        AllocateAnyPages,
        EfiLoaderData,
        (KERNEL_STACK_SIZE + 0xFFF) / 0x1000,
        &kernel_stack
    );

    UINT64 stack_top = kernel_stack + KERNEL_STACK_SIZE;

    Print(L"[loader] kernel stack: %lx - %lx\r\n", kernel_stack, stack_top);

    /* ===============================
     * Memory map (FINAL)
     * =============================== */

    UINTN map_buf_size = 4096 * 8;
    EFI_MEMORY_DESCRIPTOR* mem_map_buf;
    gBS->AllocatePool(EfiLoaderData, map_buf_size, (void**)&mem_map_buf);

    UINTN max_entries = map_buf_size / sizeof(EFI_MEMORY_DESCRIPTOR);
    KernelMemoryRegion* kmem;
    gBS->AllocatePool(EfiLoaderData,
                      max_entries * sizeof(KernelMemoryRegion),
                      (void**)&kmem);

    /* ===============================
     * Compute entry point + stack top BEFORE exiting boot services.
     *
     * All logging must happen here, while the console (a boot service)
     * is still alive. Nothing below the ExitBootServices loop may call
     * Print / gBS / gOut.
     * =============================== */

    UINT64 entry_addr = kernel_base + (ehdr->entry - min_vaddr);

    /* 16-byte align. We enter the kernel with `call`, which pushes an
       8-byte return address, so at kernel entry rsp % 16 == 8 exactly
       as the System V AMD64 ABI expects. */
    UINT64 rsp = stack_top & ~0xFULL;

    Print(L"[loader] entry: %lx\r\n", entry_addr);
    Print(L"[loader] aligned rsp: %lx\r\n", rsp);
    Print(L"[loader] exiting boot services...\r\n");

    /* ===============================
     * Get memory map + exit boot services
     *
     * CRITICAL: between GetMemoryMap and ExitBootServices there must be
     * NO boot-service or console call (no Print!). Any such call can
     * allocate and mutate the memory map, staling map_key and making
     * ExitBootServices return EFI_INVALID_PARAMETER.
     *
     * On a stale key we re-fetch the map and retry. Building the kernel
     * memory-region array is pure memory work, so it is safe inside the
     * loop. kmem was sized for max_entries >= entry_count, so no overflow.
     * =============================== */

    UINTN map_key = 0;
    UINTN desc_size = 0;
    UINT32 desc_version = 0;

    while (1) {
        UINTN map_size = map_buf_size;

        status = gBS->GetMemoryMap(
            &map_size,
            mem_map_buf,
            &map_key,
            &desc_size,
            &desc_version
        );

        if (status != EFI_SUCCESS) {
            /* Boot services are still up here, so Print is safe. */
            Print(L"[loader] GetMemoryMap failed: %x\r\n", status);
            for (;;) __asm__("hlt");
        }

        UINTN entry_count = map_size / desc_size;

        for (UINTN i = 0; i < entry_count; i++) {
            EFI_MEMORY_DESCRIPTOR* d =
                (EFI_MEMORY_DESCRIPTOR*)((UINT8*)mem_map_buf + i * desc_size);

            kmem[i].base   = d->PhysicalStart;
            kmem[i].length = d->NumberOfPages * 4096;

            switch (d->Type) {
                case EfiConventionalMemory: kmem[i].type = MEM_USABLE; break;
                case EfiACPIReclaimMemory:
                case EfiACPIMemoryNVS:      kmem[i].type = MEM_ACPI; break;
                case EfiMemoryMappedIO:     kmem[i].type = MEM_MMIO; break;
                default:                    kmem[i].type = MEM_RESERVED; break;
            }
        }

        bi->mem_map = kmem;
        bi->mem_map_entries = entry_count;

        status = gBS->ExitBootServices(gImageHandle, map_key);

        if (status == EFI_SUCCESS)
            break;

        /* Anything other than a stale key is a real failure. */
        if (status != EFI_INVALID_PARAMETER)
            for (;;) __asm__("hlt");

        /* Stale key: loop, re-fetch the map, retry. No Print here. */
    }

    /* ===============================
     * Boot services are gone.
     * Do NOT call Print / gBS / gOut past this point.
     * =============================== */

    __asm__ volatile (
        "cli\n"
        "mov %0, %%rdi\n"   // BootInfo* -> first SysV arg
        "mov %1, %%rsp\n"   // switch to kernel stack (16-aligned)
        "xor %%rbp, %%rbp\n"
        "call *%2\n"        // call kernel entry (pushes ret addr -> rsp%16==8)
        :
        : "r"(bi), "r"(rsp), "r"(entry_addr)
        : "memory"
    );

    for (;;) __asm__("hlt");
}

/* ===============================
   Command Table
   =============================== */

    SHELL_COMMAND command_table[] = {
        { L"load", cmd_loadkernel, L"load and execute kernel" },
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
