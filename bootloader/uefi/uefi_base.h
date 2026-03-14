#pragma once

#ifndef UEFI_BASE_H
#define UEFI_BASE_H


#include <stdint.h>
#include <efi.h>

#define KERNEL_PATH L"kernel.elf"

#define PAGE_SIZE 4096

/* GUIDs */
extern EFI_GUID gEfiLoadedImageProtocolGuid;
extern EFI_GUID gEfiSimpleFileSystemProtocolGuid;
extern EFI_GUID gEfiFileInfoGuid;
extern EFI_GUID gEfiGraphicsOutputProtocolGuid;

/* Global Boot Services */
extern EFI_BOOT_SERVICES *gBS;

/* Global reference to the image and system table */
extern EFI_HANDLE gImageHandle;
extern EFI_SYSTEM_TABLE *gST;

/* ===============================
   Init Functions
   =============================== */
void uefi_init(EFI_SYSTEM_TABLE *SystemTable);
void InitializeLib(EFI_HANDLE image, EFI_SYSTEM_TABLE *st);

/* ===============================
   Memory Functions
   =============================== */
void* memset(void* dst, int v, UINTN n);
void* memcpy(void* dst, const void* src, UINTN n);
void* AllocatePool(UINTN size);
void* FreePool(void* buf);

/* ===============================
   Input Functions
   =============================== */
int readline(CHAR16* buffer, int max);

/* ===============================
   Output Functions
   =============================== */
void clear(void);
void Print(CHAR16 *str, ...);

/* ===============================
   File Functions
   =============================== */
EFI_FILE* open_root(EFI_HANDLE image, EFI_SYSTEM_TABLE* st);
EFI_FILE* open_file(EFI_FILE* dir, CHAR16* path);
EFI_STATUS read_file(EFI_FILE* file, void** buffer, UINTN* size);

/* ===============================
   Memory Map
   =============================== */
EFI_STATUS get_memory_map(
    EFI_MEMORY_DESCRIPTOR** map,
    UINTN* map_size,
    UINTN* map_key,
    UINTN* desc_size,
    UINT32* desc_version
);

#endif
