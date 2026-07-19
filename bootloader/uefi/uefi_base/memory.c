#include "uefi_base.h"

void *AllocatePool(UINTN size) {
    EFI_STATUS status;
    void *buf;

    status = gBS->AllocatePool(EfiLoaderData, size, &buf);

    if (status != EFI_SUCCESS || !buf) {
        Print(L"Memory allocation failed: %x\r\n", status);
        return NULL;
    }

    return buf;
}

void *memset(void *dst, int v, UINTN n) {
    UINT8 *d = dst;
    while (n--)
        *d++ = (UINT8)v;
    return dst;
}

void *memcpy(void *dst, const void *src, UINTN n) {
    UINT8 *d = dst;
    const UINT8 *s = src;
    while (n--)
        *d++ = *s++;
    return dst;
}

void *FreePool(void *buf) {
    EFI_STATUS status = gBS->FreePool(buf);
    return status == EFI_SUCCESS ? NULL : buf;
}

EFI_STATUS get_memory_map(
    EFI_MEMORY_DESCRIPTOR** map,
    UINTN* map_size,
    UINTN* map_key,
    UINTN* desc_size,
    UINT32* desc_version
) {
    *map_size = 0;
    *map_key = 0;
    *desc_size = 0;

    // First call: get size only
    EFI_STATUS status = gBS->GetMemoryMap(map_size, NULL, map_key, desc_size, desc_version);
    
    // FIX: EFI_BUFFER_TOO_SMALL is expected here. We only fail on other errors or if size is 0.
    if (status != EFI_BUFFER_TOO_SMALL && status != EFI_SUCCESS) {
        return status;
    }
    
    if (*map_size == 0) {
        return EFI_DEVICE_ERROR;
    }

    // Add extra space for new descriptors that might be added
    *map_size += (*desc_size) * 16;

    // Allocate buffer
    *map = AllocatePool(*map_size);
    
    if (*map == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    // Second call: get the actual map
    status = gBS->GetMemoryMap(
        map_size,
        *map,
        map_key,
        desc_size,
        desc_version
    );

    return status;
}