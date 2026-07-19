#include "uefi_base.h"

/* ===============================
   Globals
   =============================== */

EFI_SIMPLE_TEXT_IN_PROTOCOL *gIn;
EFI_SIMPLE_TEXT_OUT_PROTOCOL *gOut;
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
    gImageHandle = image;
    gST = st;
    uefi_init(gST);
}