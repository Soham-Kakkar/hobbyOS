#include "uefi_base.h"
#include <stdarg.h>
#include <stdint.h>

/* ===============================
   Globals
   =============================== */

static EFI_SIMPLE_TEXT_IN_PROTOCOL *gIn;
static EFI_SIMPLE_TEXT_OUT_PROTOCOL *gOut;
EFI_BOOT_SERVICES *gBS;
EFI_HANDLE gImageHandle;
EFI_SYSTEM_TABLE *gST;

EFI_GUID gEfiLoadedImageProtocolGuid = {0x5B1B31A1,0x9562,0x11d2,{0x8E,0x3F,0x00,0xA0,0xC9,0x69,0x72,0x3B}};
EFI_GUID gEfiSimpleFileSystemProtocolGuid = {0x964e5b22,0x6459,0x11d2,{0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}};
EFI_GUID gEfiFileInfoGuid = {0x09576e92,0x6d3f,0x11d2,{0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}};
EFI_GUID gEfiGraphicsOutputProtocolGuid = {0x9042a9de,0x23dc,0x4a38,{0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a}};


/* ===============================
   Init
   =============================== */

void uefi_init(EFI_SYSTEM_TABLE *SystemTable) {
    gIn = SystemTable->ConIn;
    gOut = SystemTable->ConOut;
    gBS = SystemTable->BootServices;
}

void InitializeLib(EFI_HANDLE image, EFI_SYSTEM_TABLE *st) {
    uefi_init(st);
}


/* ===============================
   Memory / Utils
   =============================== */

void *AllocatePool(UINTN size) {
    EFI_STATUS status;
    void *buf;

    status = gBS->AllocatePool(EfiLoaderData, size, &buf);
    return (status != EFI_SUCCESS) ? NULL : buf;
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

/* ===============================
   Input
   =============================== */
static void flush_input(void) {
    EFI_INPUT_KEY key;

    while (gIn->ReadKeyStroke(gIn, &key) == EFI_SUCCESS)
        ;
}

int readline(CHAR16* buffer, int max) {
    flush_input();
    int pos = 0;

    while (1) {
        EFI_INPUT_KEY key;
        UINTN idx;

        gBS->WaitForEvent(1, &gIn->WaitForKey, &idx);
        gIn->ReadKeyStroke(gIn, &key);

        if (key.UnicodeChar == L'\r') {
            buffer[pos] = 0;
            Print(L"\r\n");
            return pos;
        }

        if (key.UnicodeChar == L'\b') {
            if (pos > 0) {
                pos--;
                Print(L"\b \b");
            }
            continue;
        }

        if (pos < max - 1) {
            buffer[pos++] = key.UnicodeChar;

            CHAR16 out[2];
            out[0] = key.UnicodeChar;
            out[1] = 0;

            Print(out);
        }
    }
}


/* ===============================
   Output
   =============================== */

INTN VSPrint(CHAR16 *str, UINTN size, CHAR16 *fmt, va_list args);

void Print(CHAR16 *str, ...) {
    CHAR16 buf[256];
    va_list args;

    va_start(args, str);
    VSPrint(buf, sizeof(buf), str, args);
    va_end(args);

    gOut->OutputString(gOut, buf);
}

#define putc(c) \
    if (count < size - 1) { \
        *p++ = (c); \
        count++; \
    }

INTN VSPrint(CHAR16 *str, UINTN size, CHAR16 *fmt, va_list args) {
    CHAR16 *p = str;
    UINTN count = 0;

    while (*fmt && count < size - 1) {
        if (*fmt == L'%') {
            fmt++;
            int long_flag = 0;

            if (*fmt == L'l') {
                long_flag = 1;
                fmt++;
            }

            switch (*fmt) {

            case L'd': {
                INT64 val = long_flag ? va_arg(args, INT64) : va_arg(args, INTN);
                CHAR16 tmp[32];
                INTN i = 0;
                int neg = 0;

                if (val < 0) {
                    neg = 1;
                    val = -val;
                }

                if (val == 0)
                    tmp[i++] = L'0';
                else
                    while (val > 0) {
                        tmp[i++] = L'0' + (val % 10);
                        val /= 10;
                    }

                if (neg)
                    putc(L'-');

                while (i > 0)
                    putc(tmp[--i]);

                break;
            }

            case L'u': {
                UINT64 val = long_flag ? va_arg(args, UINT64) : va_arg(args, UINTN);
                CHAR16 tmp[32];
                INTN i = 0;

                if (val == 0)
                    tmp[i++] = L'0';
                else
                    while (val > 0) {
                        tmp[i++] = L'0' + (val % 10);
                        val /= 10;
                    }

                while (i > 0)
                    putc(tmp[--i]);

                break;
            }

            case L'x':
            case L'X': {
                UINT64 val = long_flag ? va_arg(args, UINT64) : va_arg(args, UINTN);
                CHAR16 tmp[32];
                INTN i = 0;

                if (val == 0)
                    tmp[i++] = L'0';
                else
                    while (val > 0) {
                        UINTN digit = val % 16;

                        if (digit < 10)
                            tmp[i++] = L'0' + digit;
                        else
                            tmp[i++] = L'A' + digit - 10;

                        val /= 16;
                    }

                putc(L'0');
                putc(L'x');

                while (i > 0)
                    putc(tmp[--i]);

                break;
            }

            case L's': {
                CHAR16 *s = va_arg(args, CHAR16 *);
                while (*s)
                    putc(*s++);
                break;
            }

            case L'c':
                putc((CHAR16)va_arg(args, INTN));
                break;

            case L'%':
                putc(L'%');
                break;

            default:
                putc(L'?');
                break;
            }

            fmt++;
        } else {
            putc(*fmt++);
        }
    }

    *p = L'\0';
    return count;
}

void clear(void) {
    gOut->ClearScreen(gOut);
}


/* ===============================
   Files
   =============================== */

EFI_FILE *open_root(EFI_HANDLE image, EFI_SYSTEM_TABLE *st) {
    EFI_LOADED_IMAGE *loaded;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs;

    st->BootServices->HandleProtocol(image, &gEfiLoadedImageProtocolGuid, (void **)&loaded);
    st->BootServices->HandleProtocol(loaded->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void **)&fs);

    EFI_FILE *root;
    fs->OpenVolume(fs, &root);
    return root;
}

EFI_FILE *open_file(EFI_FILE *dir, CHAR16 *path) {
    EFI_FILE *file;
    EFI_STATUS status = dir->Open(dir, &file, path, EFI_FILE_MODE_READ, 0);

    if (status != EFI_SUCCESS)
        return NULL;

    return file;
}

EFI_STATUS read_file(EFI_FILE* file, void** buffer, UINTN* size) {
    EFI_FILE_INFO* info;
    UINTN info_size = SIZE_OF_EFI_FILE_INFO + 200;

    info = AllocatePool(info_size);
    if (!info) return EFI_OUT_OF_RESOURCES;

    file->GetInfo(file, &gEfiFileInfoGuid, &info_size, info);

    *size = info->FileSize;

    *buffer = AllocatePool(*size);
    if (!*buffer) {
        FreePool(info);
        return EFI_OUT_OF_RESOURCES;
    }

    file->Read(file, size, *buffer);
    
    FreePool(info);
    return EFI_SUCCESS;
}


/* ===============================
   Memory Map
   =============================== */

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